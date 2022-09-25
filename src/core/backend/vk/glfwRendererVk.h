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
    void setPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    void createFrameBuffers();
    void createCommandBuffers();
    void swapBuffers() override;
    void createSyncObjects();
    void createCommandPools();
    const std::vector<vk::Framebuffer>& frameBuffers() const;
    const std::vector<vk::CommandBuffer>& commandBuffers() const;
    const vk::CommandPool& commandPool() const;

private:
    DeviceVK* m_deviceVk;
    vk::Device m_device;
    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapChain;
    vk::CommandPool m_commandPool;
    std::shared_ptr<PipelineVk> m_pipeline;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    std::vector<vk::Framebuffer> m_swapchainFramebuffers;
    vk::Semaphore m_imageAvailableSemaphore;
    vk::Semaphore m_renderFinishedSemaphore;
    std::vector<vk::Fence> m_inflightFences;
    std::vector<vk::Fence> m_imagesInflight;
    std::size_t m_currentFrame{ 0 };
    bool m_framebufferResized{ false };
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
