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
GLFWRendererVk::GLFWRendererVk(Device* handle) :
    GLFWRenderer(handle)
{
    m_deviceVk = dynamic_cast<DeviceVk*>(handle);
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

GLFWRendererVk::~GLFWRendererVk() = default;

void GLFWRendererVk::swapBuffers()
{
    auto result = m_device.waitForFences(1, &m_inflightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    if (result != vk::Result::eSuccess)
    {
        LOG_ERROR("waitForFences failed");
    }
    uint32_t imageIndex;
    /// 从交换链获取图像
    // std::numeric_limits<uint64_t>::max()：用于禁用图像获取超时
    result = m_device.acquireNextImageKHR(m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[m_currentFrame], nullptr, &imageIndex);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        // 需要重新创建交换链
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
    /// 提交命令队列
    m_deviceVk->graphicsQueue().submit(submitInfo, m_inflightFences[m_currentFrame]);

    auto presentInfo = vk::PresentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &m_swapChain,
        .pImageIndices = &imageIndex
    };
    /// 呈现
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

    m_currentFrame = (m_currentFrame + 1) % DeviceVk::MAX_FRAMES_IN_FLIGHT;
    /// 等待逻辑设备的操作结束执行才能销毁窗口
    m_device.waitIdle();
}

void GLFWRendererVk::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    m_pipeline = std::dynamic_pointer_cast<PipelineVk>(pipeline);
}

} // namespace backend