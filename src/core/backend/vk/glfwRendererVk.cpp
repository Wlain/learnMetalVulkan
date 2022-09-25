//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVk.h"

#include "pipelineVk.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
    for (auto& cmd : m_commandBuffers)
    {
        m_device.freeCommandBuffers(m_commandPool, cmd);
    }
    m_device.destroyCommandPool(m_commandPool);
    m_pipeline = nullptr;
    m_device.destroy(m_imageAvailableSemaphore);
    m_device.destroy(m_renderFinishedSemaphore);
    m_device.destroy(m_swapChain);
    m_device.destroy();
    m_instance.destroy(m_surface);
    m_instance.destroy();
    // 销毁window
    glfwDestroyWindow(m_window);
}

void GLFWRendererVK::swapBuffers()
{
    auto imageIndex = m_device.acquireNextImageKHR(m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, {});
    vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    auto submitInfo = vk::SubmitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_imageAvailableSemaphore,
        .pWaitDstStageMask = &waitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_commandBuffers[imageIndex.value],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_renderFinishedSemaphore
    };
    m_deviceVk->graphicsQueue().submit(submitInfo, {});
    auto presentInfo = vk::PresentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &m_swapChain,
        .pImageIndices = &imageIndex.value
    };
    auto result = m_deviceVk->presentQueue().presentKHR(presentInfo);
    (void)result;
    m_device.waitIdle();
}

void GLFWRendererVK::createCommandBuffers()
{
    auto allocInfo = vk::CommandBufferAllocateInfo{
        .commandPool = m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(m_swapchainFramebuffers.size())
    };
    m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
}

void GLFWRendererVK::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    m_pipeline = std::dynamic_pointer_cast<PipelineVk>(pipeline);
}

void GLFWRendererVK::createSyncObjects()
{
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
    m_imageAvailableSemaphore = m_deviceVk->handle().createSemaphore(semaphoreCreateInfo);
    m_renderFinishedSemaphore = m_deviceVk->handle().createSemaphore(semaphoreCreateInfo);
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

void GLFWRendererVK::createCommandPools()
{
    auto queueFamilyIndices = m_deviceVk->findQueueFamilyIndices(m_deviceVk->gpu());
    auto poolInfo = vk::CommandPoolCreateInfo{
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
    };
    m_commandPool = m_device.createCommandPool(poolInfo);
}
} // namespace backend