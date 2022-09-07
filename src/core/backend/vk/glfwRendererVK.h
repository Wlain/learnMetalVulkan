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
    void swapWindow() override;
    void initInstance();
    void initDebugger();
    void initSurface();
    void initPhysicalDevice();
    void initDevice();
    void initSwapChain() override;
    void initPipeline();

private:
    void createSwapChain();

private:
    inline static std::string s_appName = "GLFW Vulkan Renderer";
    inline static std::string s_engineName = "GLFW Vulkan Renderer";

private:
    vk::UniqueInstance m_instance;
    vk::PhysicalDevice m_gpu;
    vk::UniqueSurfaceKHR m_surface;
    vk::UniqueDevice m_device;
    vk::UniqueSwapchainKHR m_swapChain;
    std::vector<vk::UniqueImageView> m_imageViews;
    std::vector<uint32_t> m_uniqueQueueFamilyIndices;
    vk::Pipeline m_pipeline;
};

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
