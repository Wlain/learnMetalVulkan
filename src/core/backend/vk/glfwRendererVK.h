//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERVK_H
#define LEARNMETALVULKAN_GLFWRENDERERVK_H
#include "deviceVk.h"
#include "pipelineVk.h"

namespace backend
{
class GLFWRendererVK : public GLFWRenderer
{
public:
    explicit GLFWRendererVK(Device* handle);
    ~GLFWRendererVK() override;

public:
    void swapBuffers() override;
    void setPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    void initCommandBuffer();
    void initSemaphore();
    void commit();

private:
    DeviceVK* m_deviceVk;
    vk::Device m_device;
    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapChain;
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    vk::Semaphore m_imageAvailableSemaphore;
    vk::Semaphore m_renderFinishedSemaphore;
    vk::Queue m_deviceQueue;
    vk::Queue m_presentQueue;
    std::shared_ptr<PipelineVk> m_pipeline;
    std::vector<vk::Framebuffer> m_frameBuffers;
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
