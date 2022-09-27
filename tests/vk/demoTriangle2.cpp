//
// Created by william on 2022/9/24.
//

#define GLFW_INCLUDE_VULKAN
#include "utils/utils.h"

#include <GLFW/glfw3.h>
#define VULKAN_HPP_NO_CONSTRUCTORS
#include "../mesh/globalMeshs.h"
#include "deviceVk.h"
#include "pipelineVk.h"

#include <optional>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

constexpr std::size_t MAX_FRAMES_IN_FLIGHT = 2;
#ifndef NDEBUG
extern VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
extern VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
extern bool checkValidationLayerSupport();
#endif

extern void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
extern std::vector<const char*> getLayers();
extern std::vector<const char*> getInstanceExtensions();
extern bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice);
extern std::vector<const char*> getDeviceExtensions();
extern vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
extern vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

vk::VertexInputBindingDescription getBindingDescription()
{
    auto bindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(TriangleVertex),
        .inputRate = vk::VertexInputRate::eVertex
    };
    return bindingDescription;
}

std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
{
    auto attributeDescriptions = std::array<vk::VertexInputAttributeDescription, 2>{
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(TriangleVertex, position) },
        vk::VertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(TriangleVertex, color) },
    };
    return attributeDescriptions;
}

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapchainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class HelloTriangleApplication
{
private:
    GLFWwindow* m_window{ nullptr };

    vk::Instance m_instance;
#ifndef NDEBUG
    vk::DebugUtilsMessengerEXT m_debugUtilsMessenger;
#endif
    vk::SurfaceKHR m_surface;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_device;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    vk::SwapchainKHR m_swapchain;
    std::vector<vk::Image> m_swapchainImages;
    vk::Format m_swapchainImageFormat = vk::Format::eUndefined;
    vk::Extent2D m_swapchainExtent;
    std::vector<vk::ImageView> m_swapchainImageViews;
    vk::RenderPass m_renderPass;
    vk::PipelineLayout m_graphicsPipelineLayout;
    vk::Pipeline m_graphicsPipeline;
    std::vector<vk::Framebuffer> m_swapchainFramebuffers;
    vk::CommandPool m_commandPool;
    vk::Buffer m_vertexBuffer;
    vk::DeviceMemory m_vertexBufferMemory;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inflightFences;
    std::vector<vk::Fence> m_imagesInflight;
    std::size_t m_currentFrame{ 0 };
    bool m_framebufferResized{ false };

public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(640, 480, "Vulkan triangle", nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
        glfwSetKeyCallback(m_window, keyCallback);
    }

    void initVulkan()
    {
        createInstance();
#ifndef NDEBUG
        setupDebugMessenger();
#endif
        createSurface();
        pickPhysicalDevice();
        createDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
            drawFrame();
        }
        m_device.waitIdle();
    }

    void cleanup()
    {
        cleanupSwapchain();
        m_device.destroyBuffer(m_vertexBuffer);
        m_device.freeMemory(m_vertexBufferMemory);
        for (auto fence : m_inflightFences)
        {
            m_device.destroyFence(fence);
        }
        for (auto semaphore : m_renderFinishedSemaphores)
        {
            m_device.destroySemaphore(semaphore);
        }
        for (auto semaphore : m_imageAvailableSemaphores)
        {
            m_device.destroySemaphore(semaphore);
        }
        m_device.destroyCommandPool(m_commandPool);
        m_device.destroy();
#ifndef NDEBUG
        m_instance.destroyDebugUtilsMessengerEXT(m_debugUtilsMessenger);
#endif
        m_instance.destroySurfaceKHR(m_surface);
        m_instance.destroy();

        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->m_framebufferResized = true;
    }

    void createInstance()
    {
#ifndef NDEBUG
        if (!checkValidationLayerSupport())
        {
            throw std::runtime_error{ "validation layer requested, but not available!" };
        }
#endif
        auto applicationInfo = vk::ApplicationInfo{
            .pApplicationName = "Vulkan Demo",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_1
        };

        auto layers = getLayers();
        auto instanceExtensions = getInstanceExtensions();
        auto instanceCreateInfo = vk::InstanceCreateInfo{
            .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()
        };

        m_instance = vk::createInstance(instanceCreateInfo);
    }

#ifndef NDEBUG
    void setupDebugMessenger()
    {
        auto severityFlags = vk::DebugUtilsMessageSeverityFlagsEXT{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        };

        auto messageTypeFlags = vk::DebugUtilsMessageTypeFlagsEXT{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        };

        auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT{
            .messageSeverity = severityFlags,
            .messageType = messageTypeFlags,
            .pfnUserCallback = debugCallback
        };

        m_debugUtilsMessenger = m_instance.createDebugUtilsMessengerEXT(createInfo);
    }
#endif

    void createSurface()
    {
        VkSurfaceKHR pSurface;
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &pSurface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
        m_surface = vk::SurfaceKHR{ pSurface };
    }

    void pickPhysicalDevice()
    {
        auto physicalDevices = m_instance.enumeratePhysicalDevices();
        if (physicalDevices.empty())
        {
            throw std::runtime_error{ "failed to find GPUs with Vulkan support!" };
        }

        for (const auto& physicalDevice : physicalDevices)
        {
            if (isDeviceSuitable(physicalDevice))
            {
                m_physicalDevice = physicalDevice;
                break;
            }
        }

        if (!m_physicalDevice)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createDevice()
    {
        QueueFamilyIndices indices = findQueueFamilyIndices(m_physicalDevice);

        auto uniqueQueueFamilyIndices = std::set<uint32_t>{
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        const float queuePriority = 1.0f;
        for (auto index : uniqueQueueFamilyIndices)
        {
            auto queueCreateInfo = vk::DeviceQueueCreateInfo{
                .queueFamilyIndex = index,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        auto physicalDeviceFeatures = vk::PhysicalDeviceFeatures{};

        auto layers = getLayers();
        auto deviceExtensions = getDeviceExtensions();
        auto createInfo = vk::DeviceCreateInfo{
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &physicalDeviceFeatures
        };

        m_device = m_physicalDevice.createDevice(createInfo);
        m_graphicsQueue = m_device.getQueue(indices.graphicsFamily.value(), 0);
        m_presentQueue = m_device.getQueue(indices.presentFamily.value(), 0);
    }

    void cleanupSwapchain()
    {
        for (auto framebuffer : m_swapchainFramebuffers)
        {
            m_device.destroyFramebuffer(framebuffer);
        }
        m_swapchainFramebuffers.clear();
        m_device.freeCommandBuffers(m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_device.destroyPipeline(m_graphicsPipeline);
        m_device.destroyPipelineLayout(m_graphicsPipelineLayout);
        m_device.destroyRenderPass(m_renderPass);
        for (auto imageView : m_swapchainImageViews)
        {
            m_device.destroyImageView(imageView);
        }
        m_swapchainImageViews.clear();
        m_device.destroySwapchainKHR(m_swapchain);
    }

    void recreateSwapchain()
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        m_device.waitIdle();

        cleanupSwapchain();

        createSwapchain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandBuffers();

        m_imagesInflight.resize(m_swapchainImages.size(), nullptr);
    }

    void createSwapchain()
    {
        SwapchainSupportDetails details = querySwapchainSupport(m_physicalDevice);

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
            .clipped = true
        };

        QueueFamilyIndices indices = findQueueFamilyIndices(m_physicalDevice);
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

        m_swapchain = m_device.createSwapchainKHR(createInfo);
        m_swapchainImages = m_device.getSwapchainImagesKHR(m_swapchain);
        m_swapchainImageFormat = surfaceFormat.format;
        m_swapchainExtent = extent;
    }

    void createImageViews()
    {
        for (const auto& image : m_swapchainImages)
        {
            auto createInfo = vk::ImageViewCreateInfo{
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

            m_swapchainImageViews.push_back(m_device.createImageView(createInfo));
        }
    }

    void createRenderPass()
    {
        auto colorAttachment = vk::AttachmentDescription{
            .format = m_swapchainImageFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        };
        auto colorAttachmentRef = vk::AttachmentReference{
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };
        auto subpass = vk::SubpassDescription{
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };
        auto dependency = vk::SubpassDependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
        };
        auto renderPassInfo = vk::RenderPassCreateInfo{
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };
        m_renderPass = m_device.createRenderPass(renderPassInfo);
    }

    void createGraphicsPipeline()
    {
        auto [vertShaderCode, fragShaderCode] = backend::Pipeline::getSpvFromGLSL(getFileContents("shaders/triangle.vert"), getFileContents("shaders/triangle.frag"));
        auto vertShaderModule = createShaderModule(vertShaderCode);
        auto fragShaderModule = createShaderModule(fragShaderCode);
        auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertShaderModule,
            .pName = "main"
        };

        auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragShaderModule,
            .pName = "main"
        };
        auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{ vertShaderStageInfo, fragShaderStageInfo };
        auto bindingDescription = getBindingDescription();
        auto attributeDescriptions = getAttributeDescriptions();
        auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };
        auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = false
        };
        auto viewport = vk::Viewport{
            .x = 0.0f,
            .y = static_cast<float>(m_swapchainExtent.height),
            .width = static_cast<float>(m_swapchainExtent.width),
            .height = -static_cast<float>(m_swapchainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        auto scissor = vk::Rect2D{
            .offset = { 0, 0 },
            .extent = m_swapchainExtent
        };
        auto viewportState = vk::PipelineViewportStateCreateInfo{
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };
        auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
            .depthClampEnable = false,
            .rasterizerDiscardEnable = false,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eNone,       // 默认值
            .frontFace = vk::FrontFace::eCounterClockwise, // 默认值
            .depthBiasEnable = false,
            .lineWidth = 1.0f
        };
        auto multisampling = vk::PipelineMultisampleStateCreateInfo{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = false
        };
        auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                              vk::ColorComponentFlagBits::eG |
                              vk::ColorComponentFlagBits::eB |
                              vk::ColorComponentFlagBits::eA
        };
        auto colorBlending = vk::PipelineColorBlendStateCreateInfo{
            .logicOpEnable = false,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }
        };

        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0
        };
        m_graphicsPipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

        auto pipelineInfo = vk::GraphicsPipelineCreateInfo{
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorBlending,
            .layout = m_graphicsPipelineLayout,
            .renderPass = m_renderPass,
            .subpass = 0
        };
        m_graphicsPipeline = m_device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo).value;

        m_device.destroyShaderModule(fragShaderModule);
        m_device.destroyShaderModule(vertShaderModule);
    }

    void createFramebuffers()
    {
        for (const auto& imageView : m_swapchainImageViews)
        {
            auto framebufferInfo = vk::FramebufferCreateInfo{
                .renderPass = m_renderPass,
                .attachmentCount = 1,
                .pAttachments = &imageView,
                .width = m_swapchainExtent.width,
                .height = m_swapchainExtent.height,
                .layers = 1
            };
            m_swapchainFramebuffers.push_back(m_device.createFramebuffer(framebufferInfo));
        }
    }

    void createCommandPool()
    {
        auto queueFamilyIndices = findQueueFamilyIndices(m_physicalDevice);
        auto poolInfo = vk::CommandPoolCreateInfo{
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
        };
        m_commandPool = m_device.createCommandPool(poolInfo);
    }

    void createVertexBuffer()
    {
        vk::DeviceSize bufferSize = sizeof(TriangleVertex) * g_triangleVertex.size();

        auto [stagingBuffer, stagingBufferMemory] = createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        auto data = m_device.mapMemory(stagingBufferMemory, {}, bufferSize, {});
        memcpy(data, g_triangleVertex.data(), static_cast<std::size_t>(bufferSize));
        m_device.unmapMemory(stagingBufferMemory);

        std::tie(m_vertexBuffer, m_vertexBufferMemory) = createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

        copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

        m_device.destroyBuffer(stagingBuffer);
        m_device.freeMemory(stagingBufferMemory);
    }

    void createCommandBuffers()
    {
        auto allocInfo = vk::CommandBufferAllocateInfo{
            .commandPool = m_commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<uint32_t>(m_swapchainFramebuffers.size())
        };
        m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);

        for (std::size_t i = 0; i < m_commandBuffers.size(); ++i)
        {
            auto beginInfo = vk::CommandBufferBeginInfo{};
            m_commandBuffers[i].begin(beginInfo);
            auto clearValue = vk::ClearValue{ .color = { .float32 = std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 1.0f } } };
            auto renderPassInfo = vk::RenderPassBeginInfo{
                .renderPass = m_renderPass,
                .framebuffer = m_swapchainFramebuffers[i],
                .renderArea = {
                    .offset = { 0, 0 },
                    .extent = m_swapchainExtent },
                .clearValueCount = 1,
                .pClearValues = &clearValue
            };
            m_commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            m_commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
            auto vertexBuffers = std::array<vk::Buffer, 1>{ m_vertexBuffer };
            auto offsets = std::array<vk::DeviceSize, 1>{ 0 };
            m_commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
            m_commandBuffers[i].draw(static_cast<std::uint32_t>(g_triangleVertex.size()), 1, 0, 0);
            m_commandBuffers[i].endRenderPass();
            m_commandBuffers[i].end();
        }
    }

    void createSyncObjects()
    {
        auto semaphoreInfo = vk::SemaphoreCreateInfo{};
        auto fenceInfo = vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_imageAvailableSemaphores.push_back(m_device.createSemaphore(semaphoreInfo));
            m_renderFinishedSemaphores.push_back(m_device.createSemaphore(semaphoreInfo));
            m_inflightFences.push_back(m_device.createFence(fenceInfo));
        }
        m_imagesInflight.resize(m_swapchainImages.size(), vk::Fence{ nullptr });
    }

    void drawFrame()
    {
        auto result = m_device.waitForFences(1, &m_inflightFences[m_currentFrame], true, std::numeric_limits<uint64_t>::max());

        uint32_t imageIndex;
        result = m_device.acquireNextImageKHR(
            m_swapchain,
            std::numeric_limits<uint64_t>::max(),
            m_imageAvailableSemaphores[m_currentFrame],
            nullptr,
            &imageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            recreateSwapchain();
            return;
        }
        else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        if (m_imagesInflight[imageIndex])
        {
            result = m_device.waitForFences(1, &m_imagesInflight[imageIndex], true, std::numeric_limits<uint64_t>::max());
        }
        m_imagesInflight[imageIndex] = m_inflightFences[m_currentFrame];

        auto waitSemaphores = std::vector<vk::Semaphore>{ m_imageAvailableSemaphores[m_currentFrame] };
        auto waitStages = std::vector<vk::PipelineStageFlags>{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        auto signalSemaphores = std::vector<vk::Semaphore>{ m_renderFinishedSemaphores[m_currentFrame] };
        auto submitInfo = vk::SubmitInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &m_commandBuffers[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores.data()
        };

        result = m_device.resetFences(1, &m_inflightFences[m_currentFrame]);
        m_graphicsQueue.submit(submitInfo, m_inflightFences[m_currentFrame]);

        auto presentInfo = vk::PresentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &imageIndex
        };

        result = m_presentQueue.presentKHR(presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            recreateSwapchain();
        }
        else if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        auto memProperties = m_physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    std::pair<vk::Buffer, vk::DeviceMemory> createBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties)
    {
        auto bufferInfo = vk::BufferCreateInfo{
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        auto buffer = m_device.createBuffer(bufferInfo);

        auto memoryRequirements = m_device.getBufferMemoryRequirements(buffer);
        auto allocInfo = vk::MemoryAllocateInfo{
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
        };

        auto bufferMemory = m_device.allocateMemory(allocInfo);

        m_device.bindBufferMemory(buffer, bufferMemory, 0);

        return { buffer, bufferMemory };
    }

    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        auto allocInfo = vk::CommandBufferAllocateInfo{
            .commandPool = m_commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };

        auto commandBuffers = m_device.allocateCommandBuffers(allocInfo);

        auto beginInfo = vk::CommandBufferBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };

        commandBuffers[0].begin(beginInfo);
        auto copyRegion = vk::BufferCopy{ .size = size };
        commandBuffers[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
        commandBuffers[0].end();

        auto submitInfo = vk::SubmitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers = commandBuffers.data()
        };
        m_graphicsQueue.submit(submitInfo);
        m_graphicsQueue.waitIdle();

        m_device.freeCommandBuffers(m_commandPool, 1, commandBuffers.data());
    }

    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code)
    {
        auto createInfo = vk::ShaderModuleCreateInfo{
            .codeSize = code.size() * sizeof(uint32_t),
            .pCode = code.data()
        };

        return m_device.createShaderModule(createInfo);
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width;
            int height;
            glfwGetFramebufferSize(m_window, &width, &height);

            auto actualExtent = vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
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

    QueueFamilyIndices findQueueFamilyIndices(vk::PhysicalDevice physicalDevice)
    {
        auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        QueueFamilyIndices indices;
        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilyProperties)
        {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.graphicsFamily = i;
            }
            if (physicalDevice.getSurfaceSupportKHR(i, m_surface))
            {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) { break; }
            ++i;
        }

        return indices;
    }

    [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice physicalDevice) const
    {
        SwapchainSupportDetails details;
        details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
        details.formats = physicalDevice.getSurfaceFormatsKHR(m_surface);
        details.presentModes = physicalDevice.getSurfacePresentModesKHR(m_surface);
        return details;
    }

    bool isDeviceSuitable(vk::PhysicalDevice physicalDevice)
    {
        auto indices = findQueueFamilyIndices(physicalDevice);
        bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
        bool swapchainAdequate{ false };
        if (extensionsSupported)
        {
            auto details = querySwapchainSupport(physicalDevice);
            swapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapchainAdequate;
    }
};

void demoTriangle2()
{
    HelloTriangleApplication app;
    app.run();
}