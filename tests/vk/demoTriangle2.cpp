//
// Created by william on 2022/9/24.
//

#define GLFW_INCLUDE_VULKAN
#include "pipeline.h"
#include "utils/utils.h"

#include <GLFW/glfw3.h>
#define VULKAN_HPP_NO_CONSTRUCTORS
#include "../mesh/globalMeshs.h"

#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

constexpr std::size_t MAX_FRAMES_IN_FLIGHT = 2;

#ifndef NDEBUG
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger)
{
    auto addr = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(addr);
    if (func)
    {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto addr = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(addr);
    if (func)
    {
        return func(instance, messenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << pCallbackData->pMessage << "\n";
    return VK_FALSE;
}
#endif

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

static vk::VertexInputBindingDescription getBindingDescription()
{
    auto bindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(TriangleVertex),
        .inputRate = vk::VertexInputRate::eVertex
    };

    return bindingDescription;
}

static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
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

class HelloTriangleApplication
{
private:
    GLFWwindow* window_{};

    vk::Instance instance_;
#ifndef NDEBUG
    vk::DebugUtilsMessengerEXT debugUtilsMessenger_;
#endif
    vk::SurfaceKHR surface_;

    vk::PhysicalDevice physicalDevice_;
    vk::Device device_;

    vk::Queue graphicsQueue_;
    vk::Queue presentQueue_;

    vk::SwapchainKHR swapchain_;
    std::vector<vk::Image> swapchainImages_;
    vk::Format swapchainImageFormat_ = vk::Format::eUndefined;
    vk::Extent2D swapchainExtent_;
    std::vector<vk::ImageView> swapchainImageViews_;

    vk::RenderPass renderPass_;
    vk::PipelineLayout graphicsPipelineLayout_;
    vk::Pipeline graphicsPipeline_;

    std::vector<vk::Framebuffer> swapchainFramebuffers_;
    vk::CommandPool commandPool_;

    vk::Buffer vertexBuffer_;
    vk::DeviceMemory vertexBufferMemory_;

    std::vector<vk::CommandBuffer> commandBuffers_;

    std::vector<vk::Semaphore> imageAvailableSemaphores_;
    std::vector<vk::Semaphore> renderFinishedSemaphores_;
    std::vector<vk::Fence> inflightFences_;
    std::vector<vk::Fence> imagesInflight_;
    std::size_t currentFrame_ = 0;

    bool framebufferResized = false;

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

        window_ = glfwCreateWindow(640, 480, "Vulkan triangle", nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
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
        while (!glfwWindowShouldClose(window_))
        {
            glfwPollEvents();
            drawFrame();
        }
        device_.waitIdle();
    }

    void cleanup()
    {
        cleanupSwapchain();
        device_.destroyBuffer(vertexBuffer_);
        device_.freeMemory(vertexBufferMemory_);
        for (auto fence : inflightFences_)
        {
            device_.destroyFence(fence);
        }
        for (auto semaphore : renderFinishedSemaphores_)
        {
            device_.destroySemaphore(semaphore);
        }
        for (auto semaphore : imageAvailableSemaphores_)
        {
            device_.destroySemaphore(semaphore);
        }
        device_.destroyCommandPool(commandPool_);
        device_.destroy();
#ifndef NDEBUG
        instance_.destroyDebugUtilsMessengerEXT(debugUtilsMessenger_);
#endif
        instance_.destroySurfaceKHR(surface_);
        instance_.destroy();

        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
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

        instance_ = vk::createInstance(instanceCreateInfo);
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

        debugUtilsMessenger_ = instance_.createDebugUtilsMessengerEXT(createInfo);
    }
#endif

    void createSurface()
    {
        VkSurfaceKHR pSurface;
        if (glfwCreateWindowSurface(instance_, window_, nullptr, &pSurface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
        surface_ = vk::SurfaceKHR{ pSurface };
    }

    void pickPhysicalDevice()
    {
        auto physicalDevices = instance_.enumeratePhysicalDevices();
        if (physicalDevices.empty())
        {
            throw std::runtime_error{ "failed to find GPUs with Vulkan support!" };
        }

        for (const auto& physicalDevice : physicalDevices)
        {
            if (isDeviceSuitable(physicalDevice))
            {
                physicalDevice_ = physicalDevice;
                break;
            }
        }

        if (!physicalDevice_)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createDevice()
    {
        QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice_);

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

        device_ = physicalDevice_.createDevice(createInfo);
        graphicsQueue_ = device_.getQueue(indices.graphicsFamily.value(), 0);
        presentQueue_ = device_.getQueue(indices.presentFamily.value(), 0);
    }

#ifndef NDEBUG
    static bool checkValidationLayerSupport()
    {
        auto layerProperties = vk::enumerateInstanceLayerProperties();
        return std::any_of(std::begin(layerProperties), std::end(layerProperties), [](const auto& property) {
            return std::strcmp("VK_LAYER_KHRONOS_validation", property.layerName) == 0;
        });
    }
#endif

    void cleanupSwapchain()
    {
        for (auto framebuffer : swapchainFramebuffers_)
        {
            device_.destroyFramebuffer(framebuffer);
        }
        swapchainFramebuffers_.clear();
        device_.freeCommandBuffers(commandPool_, static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
        device_.destroyPipeline(graphicsPipeline_);
        device_.destroyPipelineLayout(graphicsPipelineLayout_);
        device_.destroyRenderPass(renderPass_);
        for (auto imageView : swapchainImageViews_)
        {
            device_.destroyImageView(imageView);
        }
        swapchainImageViews_.clear();
        device_.destroySwapchainKHR(swapchain_);
    }

    void recreateSwapchain()
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window_, &width, &height);
            glfwWaitEvents();
        }

        device_.waitIdle();

        cleanupSwapchain();

        createSwapchain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandBuffers();

        imagesInflight_.resize(swapchainImages_.size(), nullptr);
    }

    void createSwapchain()
    {
        SwapchainSupportDetails details = querySwapchainSupport(physicalDevice_);

        auto surfaceFormat = chooseSwapSurfaceFormat(details.formats);
        auto presentMode = chooseSwapPresentMode(details.presentModes);
        auto extent = chooseSwapExtent(details.capabilities);

        uint32_t imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
        {
            imageCount = details.capabilities.maxImageCount;
        }

        auto createInfo = vk::SwapchainCreateInfoKHR{
            .surface = surface_,
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

        QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice_);
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

        swapchain_ = device_.createSwapchainKHR(createInfo);
        swapchainImages_ = device_.getSwapchainImagesKHR(swapchain_);
        swapchainImageFormat_ = surfaceFormat.format;
        swapchainExtent_ = extent;
    }

    void createImageViews()
    {
        for (const auto& image : swapchainImages_)
        {
            auto createInfo = vk::ImageViewCreateInfo{
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = swapchainImageFormat_,
                .components = {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity },
                .subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
            };

            swapchainImageViews_.push_back(device_.createImageView(createInfo));
        }
    }

    void createRenderPass()
    {
        auto colorAttachment = vk::AttachmentDescription{
            .format = swapchainImageFormat_,
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
        renderPass_ = device_.createRenderPass(renderPassInfo);
    }

    void createGraphicsPipeline()
    {
        auto [vertShaderCode, fragShaderCode] = backend::Pipeline::getSpvFromGLSL(getFileContents("shaders/triangle.vert"),
                                                                                  getFileContents("shaders/triangle.frag"));
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
            .primitiveRestartEnable = VK_FALSE
        };
        auto viewport = vk::Viewport{
            .x = 0.0f,
            .y = static_cast<float>(swapchainExtent_.height),
            .width = static_cast<float>(swapchainExtent_.width),
            .height = -static_cast<float>(swapchainExtent_.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        auto scissor = vk::Rect2D{
            .offset = { 0, 0 },
            .extent = swapchainExtent_
        };
        auto viewportState = vk::PipelineViewportStateCreateInfo{
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };
        auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eNone,       // 默认值
            .frontFace = vk::FrontFace::eCounterClockwise, // 默认值
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f
        };
        auto multisampling = vk::PipelineMultisampleStateCreateInfo{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = VK_FALSE
        };
        auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
            .blendEnable = VK_FALSE,
            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                              vk::ColorComponentFlagBits::eG |
                              vk::ColorComponentFlagBits::eB |
                              vk::ColorComponentFlagBits::eA
        };
        auto colorBlending = vk::PipelineColorBlendStateCreateInfo{
            .logicOpEnable = VK_FALSE,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }
        };

        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0
        };
        graphicsPipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo);

        auto pipelineInfo = vk::GraphicsPipelineCreateInfo{
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorBlending,
            .layout = graphicsPipelineLayout_,
            .renderPass = renderPass_,
            .subpass = 0
        };
        graphicsPipeline_ = device_.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo).value;

        device_.destroyShaderModule(fragShaderModule);
        device_.destroyShaderModule(vertShaderModule);
    }

    void createFramebuffers()
    {
        for (const auto& imageView : swapchainImageViews_)
        {
            auto framebufferInfo = vk::FramebufferCreateInfo{
                .renderPass = renderPass_,
                .attachmentCount = 1,
                .pAttachments = &imageView,
                .width = swapchainExtent_.width,
                .height = swapchainExtent_.height,
                .layers = 1
            };
            swapchainFramebuffers_.push_back(device_.createFramebuffer(framebufferInfo));
        }
    }

    void createCommandPool()
    {
        auto queueFamilyIndices = findQueueFamilyIndices(physicalDevice_);
        auto poolInfo = vk::CommandPoolCreateInfo{
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
        };
        commandPool_ = device_.createCommandPool(poolInfo);
    }

    void createVertexBuffer()
    {
        vk::DeviceSize bufferSize = sizeof(TriangleVertex) * g_triangleVertex.size();

        auto [stagingBuffer, stagingBufferMemory] = createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        auto data = device_.mapMemory(stagingBufferMemory, {}, bufferSize, {});
        memcpy(data, g_triangleVertex.data(), static_cast<std::size_t>(bufferSize));
        device_.unmapMemory(stagingBufferMemory);

        std::tie(vertexBuffer_, vertexBufferMemory_) = createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

        copyBuffer(stagingBuffer, vertexBuffer_, bufferSize);

        device_.destroyBuffer(stagingBuffer);
        device_.freeMemory(stagingBufferMemory);
    }

    void createCommandBuffers()
    {
        auto allocInfo = vk::CommandBufferAllocateInfo{
            .commandPool = commandPool_,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<uint32_t>(swapchainFramebuffers_.size())
        };
        commandBuffers_ = device_.allocateCommandBuffers(allocInfo);

        for (std::size_t i = 0; i < commandBuffers_.size(); ++i)
        {
            auto beginInfo = vk::CommandBufferBeginInfo{};
            commandBuffers_[i].begin(beginInfo);
            auto clearValue = vk::ClearValue{ .color = { .float32 = std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 1.0f } } };
            auto renderPassInfo = vk::RenderPassBeginInfo{
                .renderPass = renderPass_,
                .framebuffer = swapchainFramebuffers_[i],
                .renderArea = {
                    .offset = { 0, 0 },
                    .extent = swapchainExtent_ },
                .clearValueCount = 1,
                .pClearValues = &clearValue
            };
            commandBuffers_[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline_);
            auto vertexBuffers = std::array<vk::Buffer, 1>{ vertexBuffer_ };
            auto offsets = std::array<vk::DeviceSize, 1>{ 0 };
            commandBuffers_[i].bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
            commandBuffers_[i].draw(static_cast<std::uint32_t>(g_triangleVertex.size()), 1, 0, 0);
            commandBuffers_[i].endRenderPass();
            commandBuffers_[i].end();
        }
    }

    void createSyncObjects()
    {
        auto semaphoreInfo = vk::SemaphoreCreateInfo{};
        auto fenceInfo = vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            imageAvailableSemaphores_.push_back(device_.createSemaphore(semaphoreInfo));
            renderFinishedSemaphores_.push_back(device_.createSemaphore(semaphoreInfo));
            inflightFences_.push_back(device_.createFence(fenceInfo));
        }
        imagesInflight_.resize(swapchainImages_.size(), vk::Fence{ nullptr });
    }

    void drawFrame()
    {
        auto result = device_.waitForFences(
            1,
            &inflightFences_[currentFrame_],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());

        uint32_t imageIndex;
        result = device_.acquireNextImageKHR(
            swapchain_,
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores_[currentFrame_],
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

        if (imagesInflight_[imageIndex])
        {
            result = device_.waitForFences(1, &imagesInflight_[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        }
        imagesInflight_[imageIndex] = inflightFences_[currentFrame_];

        auto waitSemaphores = std::vector<vk::Semaphore>{ imageAvailableSemaphores_[currentFrame_] };
        auto waitStages = std::vector<vk::PipelineStageFlags>{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        auto signalSemaphores = std::vector<vk::Semaphore>{ renderFinishedSemaphores_[currentFrame_] };
        auto submitInfo = vk::SubmitInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers_[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores.data()
        };

        result = device_.resetFences(1, &inflightFences_[currentFrame_]);
        graphicsQueue_.submit(submitInfo, inflightFences_[currentFrame_]);

        auto presentInfo = vk::PresentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = &swapchain_,
            .pImageIndices = &imageIndex
        };

        result = presentQueue_.presentKHR(presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
        {
            framebufferResized = false;
            recreateSwapchain();
        }
        else if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        auto memProperties = physicalDevice_.getMemoryProperties();

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

        auto buffer = device_.createBuffer(bufferInfo);

        auto memoryRequirements = device_.getBufferMemoryRequirements(buffer);
        auto allocInfo = vk::MemoryAllocateInfo{
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
        };

        auto bufferMemory = device_.allocateMemory(allocInfo);

        device_.bindBufferMemory(buffer, bufferMemory, 0);

        return { buffer, bufferMemory };
    }

    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        auto allocInfo = vk::CommandBufferAllocateInfo{
            .commandPool = commandPool_,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };

        auto commandBuffers = device_.allocateCommandBuffers(allocInfo);

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
        graphicsQueue_.submit(submitInfo);
        graphicsQueue_.waitIdle();

        device_.freeCommandBuffers(commandPool_, 1, commandBuffers.data());
    }

    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code)
    {
        auto createInfo = vk::ShaderModuleCreateInfo{
            .codeSize = code.size() * sizeof(uint32_t),
            .pCode = code.data()
        };

        return device_.createShaderModule(createInfo);
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
            glfwGetFramebufferSize(window_, &width, &height);

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

    bool isDeviceSuitable(vk::PhysicalDevice physicalDevice)
    {
        QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice);
        bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            auto details = querySwapchainSupport(physicalDevice);
            swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
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
            if (physicalDevice.getSurfaceSupportKHR(i, surface_))
            {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) { break; }
            ++i;
        }

        return indices;
    }

    static std::vector<const char*> getDeviceExtensions()
    {
        return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            "VK_KHR_portability_subset"
        };
    }

    static bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice)
    {
        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        auto deviceExtensions = getDeviceExtensions();
        auto requiredExtensions = std::unordered_set<std::string>{ deviceExtensions.cbegin(), deviceExtensions.cend() };

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice physicalDevice) const
    {
        SwapchainSupportDetails details;
        details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface_);
        details.formats = physicalDevice.getSurfaceFormatsKHR(surface_);
        details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface_);
        return details;
    }

    static std::vector<char> readFile(const std::string& filename)
    {
        auto file = std::ifstream{ filename, std::ios::ate | std::ios::binary };

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }
        auto fileSize = file.tellg();
        auto buffer = std::vector<char>(static_cast<std::size_t>(fileSize));

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

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
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        auto extensions = std::vector<const char*>{ glfwExtensions, glfwExtensions + glfwExtensionCount };
#ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        return extensions;
    }
};

int demoTriangle2()
{
    HelloTriangleApplication app;
    app.run();
}