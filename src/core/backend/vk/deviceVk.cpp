//
// Created by cwb on 2022/9/8.
//

#include "deviceVk.h"

#include "commonMacro.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
namespace backend
{
static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                              void* pUserData)
{
    LOG_ERROR("validation layer: {}", pCallbackData->pMessage);
    return VK_FALSE;
}

DeviceVK::~DeviceVK() = default;

DeviceVK::DeviceVK(const Info& info) :
    Device(info)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void DeviceVK::init()
{
    Device::init();
    initInstance();
    initDebugger();
    initSurface();
    initPhysicalDevice();
    initDevice();
    creatSwapChain();
}

const vk::Instance& DeviceVK::instance() const
{
    return m_instance;
}

const vk::PhysicalDevice& DeviceVK::gpu() const
{
    return m_gpu;
}

const vk::Device& DeviceVK::handle() const
{
    return m_device;
}

void DeviceVK::initDevice()
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
        if (m_gpu.getSurfaceSupportKHR(static_cast<uint32_t>(i), m_surface))
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
    m_device = m_gpu.createDevice({ vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(), 0u, nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data() });
}

void DeviceVK::initInstance()
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
        .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
        .setPEnabledLayerNames(layers);
    m_instance = vk::createInstance(instanceCreateInfo);
}

void DeviceVK::initDebugger()
{
    // vk::DispatchLoaderDynamic dldi(*instance);
    auto dldi = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
    auto messenger = m_instance.createDebugUtilsMessengerEXTUnique(
        vk::DebugUtilsMessengerCreateInfoEXT{ {},
                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
                                              vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                              debugCallback },
        nullptr, dldi);
    (void)messenger;
}

void DeviceVK::initSurface()
{
    glfwCreateWindowSurface(m_instance, m_window, nullptr, (VkSurfaceKHR*)(&m_surface));
}

void DeviceVK::initPhysicalDevice()
{
    /// 挑选一块显卡
    // 创建 gpu
    auto gpus = m_instance.enumeratePhysicalDevices();
    // 打印显卡名
    for (auto& gpu : gpus)
    {
        LOG_INFO("gpu:{}", gpu.getProperties().deviceName);
    }
    m_gpu = gpus.front();
}

void DeviceVK::creatSwapChain()
{
    uint32_t imageCount = 2;
    SharingModeUtil sharingModeUtil{ (m_uniqueQueueFamilyIndices.begin() != m_uniqueQueueFamilyIndices.end()) ?
                                         SharingModeUtil{ vk::SharingMode::eConcurrent, 2u, m_uniqueQueueFamilyIndices.data() } :
                                         SharingModeUtil{ vk::SharingMode::eExclusive, 0u, static_cast<uint32_t*>(nullptr) } };
    auto format = vk::Format::eB8G8R8A8Unorm;
    vk::SwapchainCreateInfoKHR swapChainCreateInfo(
        {},
        m_surface,
        imageCount,
        format,
        vk::ColorSpaceKHR::eSrgbNonlinear,
        { (uint32_t)m_info.width, (uint32_t)m_info.height },
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

    m_swapChain = m_device.createSwapchainKHR(swapChainCreateInfo);
    auto swapChainImages = m_device.getSwapchainImagesKHR(m_swapChain);
    m_imageViews.reserve(swapChainImages.size());
    for (auto image : swapChainImages)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image,
                                                    vk::ImageViewType::e2D, format,
                                                    vk::ComponentMapping{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                                                                          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA },
                                                    vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        m_imageViews.emplace_back(m_device.createImageView(imageViewCreateInfo));
    }
}

const vk::SwapchainKHR& DeviceVK::swapChain() const
{
    return m_swapChain;
}

const vk::SurfaceKHR& DeviceVK::surface() const
{
    return m_surface;
}

const std::vector<uint32_t>& DeviceVK::uniqueQueueFamilyIndices() const
{
    return m_uniqueQueueFamilyIndices;
}

const std::vector<vk::ImageView>& DeviceVK::imageViews() const
{
    return m_imageViews;
}
} // namespace backend