//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERVK_H
#define LEARNMETALVULKAN_GLFWRENDERERVK_H
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include "glfwRenderer.h"

class GLFWRendererVK : public GLFWRenderer
{
public:
    struct SharingModeUtil
    {
        vk::SharingMode sharingMode;
        uint32_t familyIndicesCount;
        uint32_t* familyIndicesDataPtr;
    };

public:
    using GLFWRenderer::GLFWRenderer;
    ~GLFWRendererVK() override;

public:
    void initGlfw() override;
    void swapBuffers() override;
    void initInstance();
    void initDebugger();
    void initSurface();
    void initPhysicalDevice();
    void initDevice();
    void initSwapChain() override;
    void initPipeline(std::string_view vertShader, std::string_view fragShader);
    void initProgram(std::string_view vertShader, std::string_view fragShader);
    void initVertexBuffer();
    void initCommandBuffer();
    void commit();

private:
    void createSwapChain();

private:
    inline static std::string s_appName = "GLFW Vulkan Renderer";
    inline static std::string s_engineName = "GLFW Vulkan Renderer";

private:
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

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
