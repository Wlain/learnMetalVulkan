//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERVK_H
#define LEARNMETALVULKAN_GLFWRENDERERVK_H
#include "deviceVk.h"
#include "pipelineVk.h"

namespace backend
{
class GLFWRendererVk : public GLFWRenderer
{
public:
    explicit GLFWRendererVk(Device* handle);
    ~GLFWRendererVk() override;

public:
    void setPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    void swapBuffers() override;

private:
    DeviceVk* m_deviceVk;
    vk::Device m_device;
    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapChain;
    std::shared_ptr<PipelineVk> m_pipeline;
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    std::vector<vk::Framebuffer> m_swapchainFramebuffers;
    std::vector<vk::Fence> m_inflightFences;
    std::vector<vk::Fence> m_imagesInflight;
    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::size_t m_currentFrame{ 0 };
    bool m_framebufferResized{ false };
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
