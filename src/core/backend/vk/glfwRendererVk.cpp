//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVk.h"

#include "pipelineVk.h"
#define GLFW_INCLUDE_NONE
#include "commonMacro.h"

#include <GLFW/glfw3.h>

namespace backend
{
GLFWRendererVK::GLFWRendererVK(Device* handle) :
    GLFWRenderer(handle)
{
    m_deviceVk = dynamic_cast<DeviceVK*>(handle);
    m_device = m_deviceVk->handle();
    m_swapChain = m_deviceVk->swapChain();
    m_commandPool = m_deviceVk->commandPool();
    m_commandBuffers = m_deviceVk->commandBuffers();
    m_swapchainFramebuffers = m_deviceVk->swapchainFramebuffers();
    m_inflightFences = m_deviceVk->inflightFences();
    m_imagesInflight = m_deviceVk->imagesInflight();
    m_imageAvailableSemaphores = m_deviceVk->imageAvailableSemaphores();
    m_renderFinishedSemaphores = m_deviceVk->renderFinishedSemaphores();
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
    if (result != vk::Result::eSuccess)
    {
        LOG_ERROR("waitForFences failed");
    }
    uint32_t imageIndex;
    result = m_device.acquireNextImageKHR(
        m_swapChain,
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
        if (result != vk::Result::eSuccess)
        {
            LOG_ERROR("waitForFences %d failed", imageIndex);
        }
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
    if (result != vk::Result::eSuccess)
    {
        LOG_ERROR("resetFences failed");
    }
    m_deviceVk->graphicsQueue().submit(submitInfo, m_inflightFences[m_currentFrame]);

    auto presentInfo = vk::PresentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &m_swapChain,
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

    m_currentFrame = (m_currentFrame + 1) % DeviceVK::MAX_FRAMES_IN_FLIGHT;
    m_device.waitIdle();
}

void GLFWRendererVK::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    m_pipeline = std::dynamic_pointer_cast<PipelineVk>(pipeline);
}

} // namespace backend