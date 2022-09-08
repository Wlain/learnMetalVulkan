//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERVK_H
#define LEARNMETALVULKAN_GLFWRENDERERVK_H
#include <vulkan/vulkan.hpp>
#include "deviceVk.h"
namespace backend
{
class GLFWRendererVK : public GLFWRenderer
{
public:
    explicit GLFWRendererVK(Device* handle);
    ~GLFWRendererVK() override;

public:
    void swapBuffers() override;
    void setPipeline(const Pipeline& pipeline) override;
    void initPhysicalDevice();
    void initDevice();
    void initSwapChain() override;
    void initPipeline(std::string_view vertShader, std::string_view fragShader);
    void initProgram(std::string_view vertShader, std::string_view fragShader);
    void initVertexBuffer();
    void initCommandBuffer();
    void commit();

private:
    void initInstance();
    void createSwapChain();

private:
    DeviceVK* m_handleVk;
    vk::Instance m_instance;
    vk::PhysicalDevice m_gpu;
    vk::SurfaceKHR m_surface;
    vk::Device m_device;
    vk::SwapchainKHR m_swapChain;
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    vk::Semaphore m_imageAvailableSemaphore;
    vk::Semaphore m_renderFinishedSemaphore;
    vk::Queue m_deviceQueue;
    vk::Queue m_presentQueue;
    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    std::vector<vk::Framebuffer> m_frameBuffers;
    std::vector<vk::ImageView> m_imageViews;
    vk::RenderPass m_renderPass;
    std::vector<uint32_t> m_uniqueQueueFamilyIndices;
    std::vector<vk::PipelineShaderStageCreateInfo> m_pipelineShaderStages;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    vk::Extent2D m_extent;
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
