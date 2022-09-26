//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVk.h"

#include "pipelineVk.h"
#define GLFW_INCLUDE_NONE
#include "../../../tests/mesh/globalMeshs.h"

#include <GLFW/glfw3.h>
#define MAX_FRAMES_IN_FLIGHT 2
namespace backend
{
GLFWRendererVK::GLFWRendererVK(Device* handle) :
    GLFWRenderer(handle)
{
    m_deviceVk = dynamic_cast<DeviceVK*>(handle);
    m_device = m_deviceVk->handle();
    m_instance = m_deviceVk->instance();
    m_swapChain = m_deviceVk->swapChain();
    m_surface = m_deviceVk->surface();
}

GLFWRendererVK::~GLFWRendererVK()
{
    for (auto& framebuffer : m_swapchainFramebuffers)
    {
        m_device.destroy(framebuffer);
    }
    m_swapchainFramebuffers.clear();
    m_device.freeCommandBuffers(m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    m_pipeline = nullptr;
    m_device.destroyBuffer(m_vertexBuffer);
    m_device.freeMemory(m_vertexBufferMemory);
    for (auto fence : m_inflightFences)
    {
        m_device.destroyFence(fence);
    }
    for (auto semaphore : m_renderFinishedSemaphores)
    {
        m_device.destroySemaphore(semaphore);
    }
    for (auto semaphore : m_imageAvailableSemaphores)
    {
        m_device.destroySemaphore(semaphore);
    }
    m_device.destroyCommandPool(m_commandPool);
}

void GLFWRendererVK::swapBuffers()
{
    auto result = m_device.waitForFences(1, &m_inflightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    result = m_device.acquireNextImageKHR(
        m_deviceVk->swapChain(),
        std::numeric_limits<uint64_t>::max(),
        m_imageAvailableSemaphores[m_currentFrame],
        nullptr,
        &imageIndex);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        //        recreateSwapchain();
        return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (m_imagesInflight[imageIndex])
    {
        result = m_device.waitForFences(1, &m_imagesInflight[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    m_imagesInflight[imageIndex] = m_inflightFences[m_currentFrame];

    auto waitSemaphores = std::vector<vk::Semaphore>{ m_imageAvailableSemaphores[m_currentFrame] };
    auto waitStages = std::vector<vk::PipelineStageFlags>{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
    auto signalSemaphores = std::vector<vk::Semaphore>{ m_renderFinishedSemaphores[m_currentFrame] };
    auto submitInfo = vk::SubmitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &m_commandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores.data()
    };

    result = m_device.resetFences(1, &m_inflightFences[m_currentFrame]);
    m_deviceVk->graphicsQueue().submit(submitInfo, m_inflightFences[m_currentFrame]);

    auto presentInfo = vk::PresentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &m_deviceVk->swapChain(),
        .pImageIndices = &imageIndex
    };

    result = m_deviceVk->presentQueue().presentKHR(presentInfo);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        //        recreateSwapchain();
    }
    else if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}

void GLFWRendererVK::createCommandBuffers()
{
    auto allocInfo = vk::CommandBufferAllocateInfo{
        .commandPool = m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(m_swapchainFramebuffers.size())
    };
    m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
    for (std::size_t i = 0; i < m_commandBuffers.size(); ++i)
    {
        auto beginInfo = vk::CommandBufferBeginInfo{};
        m_commandBuffers[i].begin(beginInfo);
        auto clearValue = vk::ClearValue{ .color = { .float32 = std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 1.0f } } };
        auto renderPassInfo = vk::RenderPassBeginInfo{
            .renderPass = m_pipeline->renderPass(),
            .framebuffer = m_swapchainFramebuffers[i],
            .renderArea = {
                .offset = { 0, 0 },
                .extent = m_deviceVk->swapchainExtent() },
            .clearValueCount = 1,
            .pClearValues = &clearValue
        };
        m_commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        m_commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->handle());
        auto vertexBuffers = std::array<vk::Buffer, 1>{ m_vertexBuffer };
        auto offsets = std::array<vk::DeviceSize, 1>{ 0 };
        m_commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
        m_commandBuffers[i].draw(static_cast<std::uint32_t>(g_triangleVertex.size()), 1, 0, 0);
        m_commandBuffers[i].endRenderPass();
        m_commandBuffers[i].end();
    }
}

void GLFWRendererVK::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    m_pipeline = std::dynamic_pointer_cast<PipelineVk>(pipeline);
}

void GLFWRendererVK::createSyncObjects()
{
    auto semaphoreInfo = vk::SemaphoreCreateInfo{};
    auto fenceInfo = vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_imageAvailableSemaphores.push_back(m_device.createSemaphore(semaphoreInfo));
        m_renderFinishedSemaphores.push_back(m_device.createSemaphore(semaphoreInfo));
        m_inflightFences.push_back(m_device.createFence(fenceInfo));
    }
    m_imagesInflight.resize(m_deviceVk->swapchainImages().size(), vk::Fence{ nullptr });
}

const std::vector<vk::CommandBuffer>& GLFWRendererVK::commandBuffers() const
{
    return m_commandBuffers;
}

const std::vector<vk::Framebuffer>& GLFWRendererVK::frameBuffers() const
{
    return m_swapchainFramebuffers;
}

void GLFWRendererVK::createFrameBuffers()
{
    for (const auto& imageView : m_deviceVk->swapchainImageViews())
    {
        auto framebufferInfo = vk::FramebufferCreateInfo{
            .renderPass = m_pipeline->renderPass(),
            .attachmentCount = 1,
            .pAttachments = &imageView,
            .width = m_deviceVk->swapchainExtent().width,
            .height = m_deviceVk->swapchainExtent().height,
            .layers = 1
        };
        m_swapchainFramebuffers.push_back(m_device.createFramebuffer(framebufferInfo));
    }
}

const vk::CommandPool& GLFWRendererVK::commandPool() const
{
    return m_commandPool;
}

void GLFWRendererVK::createCommandPool()
{
    auto queueFamilyIndices = m_deviceVk->findQueueFamilyIndices(m_deviceVk->gpu());
    auto poolInfo = vk::CommandPoolCreateInfo{
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
    };
    m_commandPool = m_device.createCommandPool(poolInfo);
}

void GLFWRendererVK::startEventLoop()
{
    GLFWRenderer::startEventLoop();
    m_device.waitIdle();
}
} // namespace backend