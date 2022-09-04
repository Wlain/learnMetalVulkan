//
// Created by william on 2022/9/4.
//

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

// Command Buffer:大多数图形卡里都有一个“命令缓存”的概念, 命令缓存，是驱动填入了GPU指令的一块内存。

class VulkanApplication
{
public:
    VulkanApplication(GLFWwindow* window, std::string_view name) :
        m_window(window), m_name(name)
    {
        // one for command Buffer for initialization commands, the second for command Buffer for drawing commands
        m_cmdBuffers.resize(2);
    };

    ~VulkanApplication()
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        // 销毁window
        glfwDestroyWindow(m_window);
        // 销毁device 和 instance
        vkDestroyDevice(m_device, nullptr);
        vkDestroyInstance(m_instance, nullptr);
        free(m_queueProps);
        // 退出glfwTerminate
        glfwTerminate();
    };

public:
    void initialize()
    {
        // 如果使用debugging，打开回调试层和扩展
        m_instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        // 如果想要显示任何东西，swapChain是必须的
        m_deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        // 获取创建窗口所需的实例扩展名
        uint32_t instanceExtensionCount = 0;
        const char** instanceExtensionsBuffer = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
        for (uint32_t i = 0; i < instanceExtensionCount; ++i)
        {
            m_instanceExtensions.emplace_back(instanceExtensionsBuffer[i]);
        }
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
        if (instanceExtensionCount > 0)
        {
            auto* extensions = static_cast<VkExtensionProperties*>(malloc(sizeof(VkExtensionProperties) * instanceExtensionCount));
            vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, extensions);
            for (int i = 0; i < instanceExtensionCount; i++)
            {
                if (!strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, extensions[i].extensionName))
                {
                    m_instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                }
            }
            free(extensions);
        }
        // 创建 instance
        VkApplicationInfo applicationInfo;
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.apiVersion = VK_API_VERSION_1_0;
        applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        applicationInfo.pApplicationName = m_name.c_str();
        applicationInfo.pEngineName = m_name.c_str();

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = m_instanceLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();
        instanceCreateInfo.enabledExtensionCount = m_instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensions.data();
        instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);

        //  获取gpus
        uint32_t gpuCount;
        vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
        std::vector<VkPhysicalDevice> gpus(gpuCount);
        vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data());
        m_gpu = gpus[0];

        // 选择graphics queue family
        vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &m_queueCount, nullptr);
        m_queueProps = (VkQueueFamilyProperties*)malloc(m_queueCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &m_queueCount, m_queueProps);
        for (uint32_t i = 0; i < m_graphicsQueueNodeIndex; ++i)
        {
            if (m_queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_graphicsQueueNodeIndex = i;
            }
        }
        if (m_graphicsQueueNodeIndex == UINT32_MAX)
        {
            glfwTerminate();
        }
    }

    void creatSwapChain()
    {
        // 创建窗口表面,这是针对所有操作系统的一种功能解决方案
        // Create a WSI surface for the window:
        VkResult ret = glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);
        if (VK_SUCCESS != ret)
        {
            glfwTerminate();
        }
        VkSurfaceCapabilitiesKHR surfCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surfCapabilities);

        // 创建 device
        // GraphicsQueueFamily 现在包含支持图形的队列族ID
        // 创建 Vulkan device
        const float priorities[]{ 1.0f };
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = m_graphicsQueueNodeIndex;
        queueCreateInfo.pQueuePriorities = priorities;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.enabledLayerCount = m_deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
        vkCreateDevice(m_gpu, &deviceCreateInfo, nullptr, &m_device);
        vkGetPhysicalDeviceFeatures(m_gpu, &m_gpuFeatures);

        vkGetDeviceQueue(m_device, m_graphicsQueueNodeIndex, 0, &m_queue);
        // Query with nullptr data to get count
        vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &m_queueCount, nullptr);
        assert(m_queueCount >= 1);

        //         Get the list of VkFormat's that are supported:
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, nullptr);
        auto* surfFormats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, surfFormats);
        VkFormat format = formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED ? VK_FORMAT_B8G8R8A8_UNORM : surfFormats[0].format;

        // Get Memory information and properties
        vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_memoryProperties);
        uint32_t desiredNumOfswapChainImages = surfCapabilities.minImageCount;
        VkColorSpaceKHR colorSpace = surfFormats[0].colorSpace;
        //        VkSwapchainCreateInfoKHR swapChainInfo = {
        //            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        //            .pNext = nullptr,
        //            .surface = m_surface,
        //            .minImageCount = desiredNumOfswapChainImages,
        //            .imageFormat = format,
        //            .imageColorSpace = colorSpace,
        //            .imageExtent = {
        //                .width = m_width,
        //                .height = m_width,
        //            },
        //            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        //            //        .preTransform = preTransform,
        //            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        //            .imageArrayLayers = 1,
        //            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        //            .queueFamilyIndexCount = 0,
        //            .pQueueFamilyIndices = nullptr,
        //            .presentMode = swapChainPresentMode,
        //            .oldSwapchain = nullptr,
        //            .clipped = true,
        //        };
        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = m_surface;
        swapChainCreateInfo.minImageCount = desiredNumOfswapChainImages;
        swapChainCreateInfo.imageFormat = format;
        swapChainCreateInfo.imageColorSpace = colorSpace;
        swapChainCreateInfo.imageExtent = {
            .width = m_width,
            .height = m_height,
        };
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapChainCreateInfo.clipped = true;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, nullptr, &m_swapChainKHR);
    }

    void creatCommandBuffer()
    {
        const VkCommandPoolCreateInfo cmdPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .queueFamilyIndex = m_graphicsQueueNodeIndex,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        };
        vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdBufPool);

        const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_cmdBufPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        vkAllocateCommandBuffers(m_device, &cmd, &m_cmdBuffers[0]);
    }

    void recordCommandBuffer()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        VkClearColorValue clearColor = { 1.0f, 0.0f, 0.0f, 1.0f };
        VkClearValue clearValue = {};
        clearValue.color = clearColor;
        VkImageSubresourceRange imageRange = {};
        imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageRange.levelCount = 1;
        imageRange.layerCount = 1;
        VkResult res = vkBeginCommandBuffer(m_cmdBuffers[0], &beginInfo);
        vkCmdClearColorImage(m_cmdBuffers[0], m_images[0], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);
        vkEndCommandBuffer(m_cmdBuffers[0]);
    }

    void render()
    {
        uint32_t ImageIndex = 0;
        // Get the index of the next available swapChain image:
        vkAcquireNextImageKHR(m_device, m_swapChainKHR, UINT64_MAX, nullptr, nullptr, &ImageIndex);
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_cmdBuffers[ImageIndex];
        vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChainKHR;
        presentInfo.pImageIndices = &ImageIndex;
        vkQueuePresentKHR(m_queue, &presentInfo);
    }

    const std::string& name() const
    {
        return m_name;
    }

    VkInstance& instance()
    {
        return m_instance;
    }

private:
    // 声明常规实例和设备层和扩展
    std::vector<const char*> m_instanceExtensions;
    std::vector<const char*> m_deviceExtensions;
    std::vector<const char*> m_instanceLayers;
    std::string m_name;
    GLFWwindow* m_window = nullptr;
    VkInstance m_instance{ VK_NULL_HANDLE };
    VkPhysicalDevice m_gpu{ VK_NULL_HANDLE };
    VkPhysicalDeviceMemoryProperties m_memoryProperties;
    VkPhysicalDeviceProperties m_gpuProps;
    VkPhysicalDeviceFeatures m_gpuFeatures;
    VkQueueFamilyProperties* m_queueProps;
    VkDevice m_device{ VK_NULL_HANDLE };
    VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

    std::vector<VkImage> m_images;
    VkSwapchainKHR m_swapChainKHR;
    VkQueue m_queue{ VK_NULL_HANDLE };
    uint32_t m_queueCount;
    uint32_t m_currentBuffer;
    std::vector<VkCommandBuffer> m_cmdBuffers;
    VkCommandPool m_cmdBufPool{ VK_NULL_HANDLE };
    uint32_t m_graphicsQueueNodeIndex{ UINT32_MAX };
    uint32_t m_width{ 640 };
    uint32_t m_height{ 480 };
};

void windowVK()
{
    // 初始化glfw
    glfwInit();
    // 检查 vulkan support
    if (GLFW_FALSE == glfwVulkanSupported())
    {
        glfwTerminate();
    }
    gladLoadVulkanUserPtr(nullptr, (GLADuserptrloadfunc)glfwGetInstanceProcAddress, nullptr);

    // 创建 window
    // 告诉 GLFW 不要用窗口创建 OpenGL 上下文
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    constexpr std::string_view name = "GLFW with Vulkan";
    auto* window = glfwCreateWindow(640, 480, name.data(), nullptr, nullptr);
    // 初始化 VulkanApplication
    VulkanApplication application(window, name.data());
    application.initialize();
    application.creatSwapChain();
    application.creatCommandBuffer();
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // render
        // 清屏需要的操作：
        // 从交换链中请求下一个image
        // 提交命令缓存
        // 提交“显示image”的请求
        //        application.render();
    }
}
