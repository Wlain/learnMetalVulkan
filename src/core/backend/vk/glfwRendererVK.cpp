//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVK.h"

#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void GLFWRendererVK::initInstance()
{
    // initialize the vk::ApplicationInfo structure
    vk::ApplicationInfo appInfo(s_appName.c_str(), 1, s_engineName.c_str(), 1, VK_API_VERSION_1_1);
    // initialize the vk::InstanceCreateInfo
    auto glfwExtensionCount = 0u;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + glfwExtensionCount);
    glfwExtensionsVector.emplace_back("VK_EXT_debug_utils");
    glfwExtensionsVector.emplace_back("VK_KHR_portability_enumeration");
    auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.setPApplicationInfo(&appInfo)
        .setPEnabledExtensionNames(glfwExtensionsVector)
#if defined(TARGET_OS_MAC)
        .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
#endif
        .setPEnabledLayerNames(layers);
    m_instance = vk::createInstanceUnique(instanceCreateInfo);
}

void GLFWRendererVK::initDebugger()
{
    // vk::DispatchLoaderDynamic dldi(*instance);
    auto dldi = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);
    auto messenger = m_instance->createDebugUtilsMessengerEXTUnique(
        vk::DebugUtilsMessengerCreateInfoEXT{ {},
                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
                                              vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                              debugCallback },
        nullptr, dldi);
    (void)messenger;
}

void GLFWRendererVK::initSurface()
{
    VkSurfaceKHR surfaceTmp;
    glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surfaceTmp);
    m_surface = vk::UniqueSurfaceKHR(surfaceTmp, *m_instance);
}

GLFWRendererVK::~GLFWRendererVK()
{
    // 销毁window
    glfwDestroyWindow(m_window);
}

void GLFWRendererVK::initGlfw()
{
    // This tells GLFW to not create an OpenGL context with the window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    initInstance();
    initDebugger();
}

void GLFWRendererVK::swapWindow()
{
}

void GLFWRendererVK::initSwapChain()
{
    initSurface();
    initPhysicalDevice();
    initDevice();
    createSwapChain();
    initPipeline();
}

void GLFWRendererVK::initPhysicalDevice()
{
    /// 挑选一块显卡
    // 创建 gpu
    auto gpus = m_instance->enumeratePhysicalDevices();
    // 打印显卡名
    for (auto& gpu : gpus)
    {
        std::cout << gpu.getProperties().deviceName << "\n";
    }
    m_gpu = gpus.front();
}

void GLFWRendererVK::initDevice()
{
    // 创建 Device 和 命令队列
    // 两个命令队列
    // 一个是绘制命令：queueFamilyProperties
    // 一个是显示命令: presentQueueFamilyIndex
    // get the QueueFamilyProperties of the first PhysicalDevice
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_gpu.getQueueFamilyProperties();
    // get the first index into queueFamiliyProperties which supports graphics
    size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProperties.begin(), std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(), [](vk::QueueFamilyProperties const& qfp) {
                                                        return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                                                    }));
    assert(graphicsQueueFamilyIndex < queueFamilyProperties.size());
    size_t presentQueueFamilyIndex = 0u;
    for (auto i = 0ul; i < queueFamilyProperties.size(); i++)
    {
        if (m_gpu.getSurfaceSupportKHR(static_cast<uint32_t>(i), *m_surface))
        {
            presentQueueFamilyIndex = i;
        }
    }
    m_uniqueQueueFamilyIndices = { static_cast<uint32_t>(graphicsQueueFamilyIndex),
                                   static_cast<uint32_t>(presentQueueFamilyIndex) };
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 0.0f;
    for (auto& queueFamilyIndex : m_uniqueQueueFamilyIndices)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(queueFamilyIndex), 1, &queuePriority);
    }
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset" };
    m_device = m_gpu.createDeviceUnique({ vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(), 0u, nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data() });
}

void GLFWRendererVK::createSwapChain()
{
    uint32_t imageCount = 2;
    SharingModeUtil sharingModeUtil{ (m_uniqueQueueFamilyIndices.begin() != m_uniqueQueueFamilyIndices.end()) ?
                                         SharingModeUtil{ vk::SharingMode::eConcurrent, 2u, m_uniqueQueueFamilyIndices.data() } :
                                         SharingModeUtil{ vk::SharingMode::eExclusive, 0u, static_cast<uint32_t*>(nullptr) } };
    auto format = vk::Format::eB8G8R8A8Unorm;
    auto extent = vk::Extent2D{ (uint32_t)m_windowWidth, (uint32_t)m_windowHeight };
    vk::SwapchainCreateInfoKHR swapChainCreateInfo(
        {},
        *m_surface,
        imageCount,
        format,
        vk::ColorSpaceKHR::eSrgbNonlinear,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        sharingModeUtil.sharingMode,
        sharingModeUtil.familyIndicesCount,
        sharingModeUtil.familyIndicesDataPtr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::PresentModeKHR::eFifo,
        true,
        nullptr);

    m_swapChain = m_device->createSwapchainKHRUnique(swapChainCreateInfo);
    auto swapChainImages = m_device->getSwapchainImagesKHR(*m_swapChain);
    m_imageViews.reserve(swapChainImages.size());
    for (auto image : swapChainImages)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image,
                                                    vk::ImageViewType::e2D, format,
                                                    vk::ComponentMapping{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                                                                          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA },
                                                    vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        m_imageViews.emplace_back(m_device->createImageViewUnique(imageViewCreateInfo));
    }
}

void GLFWRendererVK::initPipeline()
{
    vk::GraphicsPipelineCreateInfo info;
    // set shader config
    // vertex input
    // set Assembly
    // layout
    // viewport and Scissor
    // set rasterization
    // multiSample
    // depthStencil
    // color blend
    // renderPass
}
