//
// Created by cwb on 2022/9/8.
//

#include "deviceVk.h"

#include "commonMacro.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <set>
#include <unordered_set>

namespace backend
{
#ifndef NDEBUG
static bool checkValidationLayerSupport()
{
    auto layerProperties = vk::enumerateInstanceLayerProperties();
    return std::any_of(layerProperties.begin(), layerProperties.end(), [](const auto& property) {
        return std::strcmp("VK_LAYER_KHRONOS_validation", property.layerName) == 0;
    });
}

static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                              void* pUserData)
{
    LOG_INFO("validation layer:\n {}", pCallbackData->pMessage);
    return VK_FALSE;
}
#endif

static std::vector<const char*> getLayers()
{
    std::vector<const char*> layers;
#ifndef NDEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    return layers;
}

static std::vector<const char*> getInstanceExtensions()
{
    auto glfwExtensionCount = 0u;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifndef NDEBUG
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    return extensions;
}

static std::vector<const char*> getDeviceExtensions()
{
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset"
    };
}

static bool checkDeviceExtensionSupport(vk::PhysicalDevice gpu)
{
    auto availableExtensions = gpu.enumerateDeviceExtensionProperties();
    auto deviceExtensions = getDeviceExtensions();
    auto requiredExtensions = std::unordered_set<std::string>(deviceExtensions.cbegin(), deviceExtensions.cend());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    auto it = std::find_if(std::begin(availableFormats), std::end(availableFormats), [](vk::SurfaceFormatKHR format) {
        return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });
    return it == availableFormats.cend() ? availableFormats.front() : *it;
}

static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    auto it = std::find(std::begin(availablePresentModes), std::end(availablePresentModes), vk::PresentModeKHR::eMailbox);
    return it == availablePresentModes.cend() ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
}

vk::Extent2D DeviceVK::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        auto actualExtent = vk::Extent2D{ static_cast<uint32_t>(m_info.width), static_cast<uint32_t>(m_info.height) };
        actualExtent.width = std::clamp(
            actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(
            actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

DeviceVK::~DeviceVK()
{
    m_instance.destroyDebugUtilsMessengerEXT(m_messenger);
}

DeviceVK::DeviceVK(const Info& info) :
    Device(info)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void DeviceVK::init()
{
    Device::init();
    initInstance();
#ifndef NDEBUG
    initDebugger();
#endif
    initSurface();
    pickPhysicalDevice();
    initDevice();
    creatSwapChain();
    createImageViews();
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
    auto indices = findQueueFamilyIndices(m_gpu);
    auto uniqueQueueFamilyIndices = std::set<uint32_t>{
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 0.0f;
    for (auto& index : uniqueQueueFamilyIndices)
    {
        auto queueCreateInfo = vk::DeviceQueueCreateInfo{
            .queueFamilyIndex = index,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.emplace_back(queueCreateInfo);
    }
    auto gpuDeviceFeatures = vk::PhysicalDeviceFeatures{};
    auto layer = getLayers();
    auto deviceExtensions = getDeviceExtensions();
    auto createInfo = vk::DeviceCreateInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = static_cast<uint32_t>(layer.size()),
        .ppEnabledLayerNames = layer.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &gpuDeviceFeatures
    };
    m_device = m_gpu.createDevice(createInfo);
    m_graphicsQueue = m_device.getQueue(indices.graphicsFamily.value(), 0);
    m_presentQueue = m_device.getQueue(indices.presentFamily.value(), 0);
}

void DeviceVK::initInstance()
{
#ifndef NDEBUG
    if (!checkValidationLayerSupport())
    {
        LOG_ERROR("validation layer is requested, but not available!");
    }
#endif
    // initialize the vk::ApplicationInfo structure
    vk::ApplicationInfo appInfo{
        .pApplicationName = m_info.title.c_str(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = m_info.title.c_str(),
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1
    };
    // initialize the vk::InstanceCreateInfo
    auto layers = getLayers();
    auto instanceExtensions = getInstanceExtensions();
    auto instanceCreateInfo = vk::InstanceCreateInfo{
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data()
    };
    m_instance = vk::createInstance(instanceCreateInfo);
}
#ifndef NDEBUG
void DeviceVK::initDebugger()
{
    auto messageSeverity = vk::DebugUtilsMessageSeverityFlagsEXT{
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    };
    auto messageType = vk::DebugUtilsMessageTypeFlagsEXT{
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    };
    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity = messageSeverity,
        .messageType = messageType,
        .pfnUserCallback = debugCallback
    };
    m_messenger = m_instance.createDebugUtilsMessengerEXT(createInfo);
    (void)m_messenger;
}
#endif

void DeviceVK::initSurface()
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &surface))
    {
        LOG_ERROR("failed to create window surface!");
    }
    m_surface = surface;
}

void DeviceVK::pickPhysicalDevice()
{
    /// 挑选一块显卡
    // 创建 gpu
    auto gpus = m_instance.enumeratePhysicalDevices();
    if (gpus.empty())
    {
        LOG_ERROR("failed to find gpus with Vulkan support!");
    }
    // 打印显卡名
    for (const auto& gpu : gpus)
    {
        LOG_INFO("gpu:{}", gpu.getProperties().deviceName);
        if (isDeviceSuitable(gpu))
        {
            m_gpu = gpu;
            break;
        }
    }
    if (!m_gpu)
    {
        LOG_ERROR("failed to find a suitable GPU!");
    }
}

void DeviceVK::creatSwapChain()
{
    auto details = querySwapchainSupport(m_gpu);
    auto surfaceFormat = chooseSwapSurfaceFormat(details.formats);
    auto presentMode = chooseSwapPresentMode(details.presentModes);
    auto extent = chooseSwapExtent(details.capabilities);
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
    {
        imageCount = details.capabilities.maxImageCount;
    }
    auto createInfo = vk::SwapchainCreateInfoKHR{
        .surface = m_surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = VK_TRUE
    };
    QueueFamilyIndices indices = findQueueFamilyIndices(m_gpu);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    m_swapChain = m_device.createSwapchainKHR(createInfo);
    m_swapchainImages = m_device.getSwapchainImagesKHR(m_swapChain);
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
    m_swapchainImagesView.reserve(m_swapchainImages.size());
}

const vk::SwapchainKHR& DeviceVK::swapChain() const
{
    return m_swapChain;
}

const vk::SurfaceKHR& DeviceVK::surface() const
{
    return m_surface;
}

uint32_t DeviceVK::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    auto memProperties = m_gpu.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    LOG_ERROR("failed to find suitable memory type!");
    ASSERT(0);
    return 0;
}

bool DeviceVK::isDeviceSuitable(vk::PhysicalDevice gpu)
{
    auto indices = findQueueFamilyIndices(gpu);
    bool extensionsSupported = checkDeviceExtensionSupport(gpu);
    bool swapchainAdequate{ false };
    if (extensionsSupported)
    {
        auto details = querySwapchainSupport(gpu);
        swapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
    }
    return indices.isComplete() && extensionsSupported && swapchainAdequate;
}

DeviceVK::QueueFamilyIndices DeviceVK::findQueueFamilyIndices(vk::PhysicalDevice gpu)
{
    auto queueFamilyProperties = gpu.getQueueFamilyProperties();
    QueueFamilyIndices indices;
    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilyProperties)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }
        if (gpu.getSurfaceSupportKHR(i, m_surface))
        {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) { break; }
        ++i;
    }
    return indices;
}

DeviceVK::SwapchainSupportDetails DeviceVK::querySwapchainSupport(vk::PhysicalDevice gpu) const
{
    SwapchainSupportDetails details;
    details.capabilities = gpu.getSurfaceCapabilitiesKHR(m_surface);
    details.formats = gpu.getSurfaceFormatsKHR(m_surface);
    details.presentModes = gpu.getSurfacePresentModesKHR(m_surface);
    return details;
}

const vk::Queue& DeviceVK::graphicsQueue() const
{
    return m_graphicsQueue;
}

const vk::Queue& DeviceVK::presentQueue() const
{
    return m_presentQueue;
}

const std::vector<vk::Image>& DeviceVK::swapchainImages() const
{
    return m_swapchainImages;
}

const std::vector<vk::ImageView>& DeviceVK::swapchainImageViews() const
{
    return m_swapchainImagesView;
}

vk::Format DeviceVK::swapchainImageFormat() const
{
    return m_swapchainImageFormat;
}

const vk::Extent2D& DeviceVK::swapchainExtent() const
{
    return m_swapchainExtent;
}

void DeviceVK::createImageViews()
{
    for (auto image : m_swapchainImages)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = m_swapchainImageFormat,
            .components = {
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity },
            .subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
        };
        m_swapchainImagesView.emplace_back(m_device.createImageView(imageViewCreateInfo));
    }
}
} // namespace backend