//
// Created by william on 2022/9/4.
//

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>

void windowGlfwVK()
{
    // 声明常规实例和设备层和扩展
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
    std::vector<const char*> instanceLayers;
    // 如果使用debugging，打开回调试层和扩展
    instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    // 如果想要显示任何东西，swapChain是必须的
    deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // 初始化glfw
    glfwInit();
    // 检查 vulkan support
    if (GLFW_FALSE == glfwVulkanSupported())
    {
        glfwTerminate();
    }
    gladLoadVulkanUserPtr(nullptr, (GLADuserptrloadfunc)glfwGetInstanceProcAddress, nullptr);
    // 获取创建窗口所需的实例扩展名
    uint32_t instanceExtensionCount = 0;
    const char** instanceExtensionsBuffer = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        instanceExtensions.emplace_back(instanceExtensionsBuffer[i]);
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
                instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
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
    applicationInfo.pApplicationName = "GLFW with Vulkan";
    applicationInfo.pEngineName = "GLFW with Vulkan";

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = instanceLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
    instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    VkInstance instance;
    vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    //  获取gpus
    uint32_t gpuCount;
    vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

    // 选择graphics queue family
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, familyProperties.data());

    uint32_t graphicsQueueFamily = UINT32_MAX;
    for (uint32_t i = 0; i < graphicsQueueFamily; ++i)
    {
        if (familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamily = i;
        }
    }

    if (graphicsQueueFamily == UINT32_MAX)
    {
        glfwTerminate();
    }

    // GraphicsQueueFamily 现在包含支持图形的队列族ID
    // 创建 Vulkan device
    const float priorities[]{ 1.0f };
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
    queueCreateInfo.pQueuePriorities = priorities;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledLayerCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkDevice device = VK_NULL_HANDLE;
    vkCreateDevice(gpus[0], &deviceCreateInfo, nullptr, &device);

    // 创建 window
    int width = 800;
    int height = 600;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // This tells GLFW to not create an OpenGL context with the window
    auto window = glfwCreateWindow(width, height, applicationInfo.pApplicationName, nullptr, nullptr);

    // make sure we indeed get the surface size we want.
    glfwGetFramebufferSize(window, &width, &height);

    // 创建窗口表面,这是针对所有操作系统的一种功能解决方案
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult ret = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (VK_SUCCESS != ret)
    {
        glfwTerminate();
    }
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
    }
    // 销毁surface对象
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // 销毁window
    glfwDestroyWindow(window);

    // 销毁device 和 instance
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    // 退出glfwTerminate
    glfwTerminate();
}