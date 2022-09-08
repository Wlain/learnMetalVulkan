//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVk.h"

#include "shaderVk.h"

#include <iostream>
#include <shaderc/shaderc.hpp>

static VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
        .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
        .setPEnabledLayerNames(layers);
    m_instance = vk::createInstance(instanceCreateInfo);
}

void GLFWRendererVK::initDebugger()
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

void GLFWRendererVK::initSurface()
{
    glfwCreateWindowSurface(m_instance, m_window, nullptr, (VkSurfaceKHR*)(&m_surface));
}

GLFWRendererVK::~GLFWRendererVK()
{
    for (auto& framebuffer : m_frameBuffers)
    {
        m_device.destroy(framebuffer);
    }
    for (auto& cmd : m_commandBuffers)
    {
        m_device.freeCommandBuffers(m_commandPool, cmd);
    }
    m_device.destroyCommandPool(m_commandPool);
    m_device.destroy(m_vertexShaderModule);
    m_device.destroy(m_fragmentShaderModule);
    m_device.destroy(m_pipelineLayout);
    m_device.destroy(m_pipeline);
    m_device.destroy(m_renderPass);
    for (auto& view : m_imageViews)
    {
        m_device.destroy(view);
    }
    m_device.destroy(m_imageAvailableSemaphore);
    m_device.destroy(m_renderFinishedSemaphore);
    m_device.destroy(m_swapChain);
    m_device.destroy();
    m_instance.destroy(m_surface);
    m_instance.destroy();
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

void GLFWRendererVK::swapBuffers()
{
    auto imageIndex = m_device.acquireNextImageKHR(m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, {});
    vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    auto submitInfo = vk::SubmitInfo{ 1, &m_imageAvailableSemaphore, &waitStageMask, 1, &m_commandBuffers[imageIndex.value], 1, &m_renderFinishedSemaphore };
    m_deviceQueue.submit(submitInfo, {});
    auto presentInfo = vk::PresentInfoKHR{ 1, &m_renderFinishedSemaphore, 1, &m_swapChain, &imageIndex.value };
    auto result = m_presentQueue.presentKHR(presentInfo);
    (void)result;
    m_device.waitIdle();
}

void GLFWRendererVK::initSwapChain()
{
    initSurface();
    initPhysicalDevice();
    initDevice();
    createSwapChain();
}

void GLFWRendererVK::initPhysicalDevice()
{
    /// 挑选一块显卡
    // 创建 gpu
    auto gpus = m_instance.enumeratePhysicalDevices();
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

void GLFWRendererVK::createSwapChain()
{
    uint32_t imageCount = 2;
    SharingModeUtil sharingModeUtil{ (m_uniqueQueueFamilyIndices.begin() != m_uniqueQueueFamilyIndices.end()) ?
                                         SharingModeUtil{ vk::SharingMode::eConcurrent, 2u, m_uniqueQueueFamilyIndices.data() } :
                                         SharingModeUtil{ vk::SharingMode::eExclusive, 0u, static_cast<uint32_t*>(nullptr) } };
    auto format = vk::Format::eB8G8R8A8Unorm;
    m_extent = vk::Extent2D{ (uint32_t)m_windowWidth, (uint32_t)m_windowHeight };
    vk::SwapchainCreateInfoKHR swapChainCreateInfo(
        {},
        m_surface,
        imageCount,
        format,
        vk::ColorSpaceKHR::eSrgbNonlinear,
        m_extent,
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

void GLFWRendererVK::initPipeline(std::string_view vertShader, std::string_view fragShader)
{
    vk::GraphicsPipelineCreateInfo info;
    // set shader config
    initProgram(vertShader, fragShader);
    info.setStageCount(2).setPStages(m_pipelineShaderStages.data());
    // vertex input
    initVertexBuffer();
    info.setPVertexInputState(&m_vertexInputInfo);
    // set Assembly
    auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{ {}, vk::PrimitiveTopology::eTriangleList, false };
    info.setPInputAssemblyState(&inputAssembly);
    // layout
    m_pipelineLayout = m_device.createPipelineLayout({}, nullptr);
    info.setLayout(m_pipelineLayout);
    // viewport and Scissor
    auto viewport = vk::Viewport{ 0.0f, 0.0f, static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), 0.0f, 1.0f };
    auto scissor = vk::Rect2D{ { 0, 0 }, m_extent };
    auto viewportState = vk::PipelineViewportStateCreateInfo{ {}, 1, &viewport, 1, &scissor };
    info.setPViewportState(&viewportState);
    // set rasterization
    auto rasterizer = vk::PipelineRasterizationStateCreateInfo{ {}, /*depthClamp*/ false,
                                                                /*rasterizeDiscard*/ false,
                                                                vk::PolygonMode::eFill,
                                                                {},
                                                                /*frontFace*/ vk::FrontFace::eCounterClockwise,
                                                                {},
                                                                {},
                                                                {},
                                                                {},
                                                                1.0f };
    info.setPRasterizationState(&rasterizer);
    // multiSample
    auto multisampling = vk::PipelineMultisampleStateCreateInfo{ {}, vk::SampleCountFlagBits::e1, false, 1.0 };
    info.setPMultisampleState(&multisampling);
    // depthStencil
    info.setPDepthStencilState(nullptr);
    // color blend
    auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{ {}, /*srcCol*/ vk::BlendFactor::eOne,
                                                                       /*dstCol*/ vk::BlendFactor::eZero,
                                                                       /*colBlend*/ vk::BlendOp::eAdd,
                                                                       /*srcAlpha*/ vk::BlendFactor::eOne,
                                                                       /*dstAlpha*/ vk::BlendFactor::eZero,
                                                                       /*alphaBlend*/ vk::BlendOp::eAdd,
                                                                       vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };

    auto colorBlending = vk::PipelineColorBlendStateCreateInfo{ {},
                                                                /*logicOpEnable=*/false,
                                                                vk::LogicOp::eCopy,
                                                                /*attachmentCount=*/1,
                                                                /*colourAttachments=*/&colorBlendAttachment };
    info.setPColorBlendState(&colorBlending);
    // renderPass
    auto format = vk::Format::eB8G8R8A8Unorm;
    auto colorAttachment = vk::AttachmentDescription{ {}, format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, {}, vk::ImageLayout::ePresentSrcKHR };
    auto colourAttachmentRef = vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };
    auto subpass = vk::SubpassDescription{ {},
                                           vk::PipelineBindPoint::eGraphics,
                                           /*inAttachmentCount*/ 0,
                                           nullptr,
                                           1,
                                           &colourAttachmentRef };
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
    m_imageAvailableSemaphore = m_device.createSemaphore(semaphoreCreateInfo);
    m_renderFinishedSemaphore = m_device.createSemaphore(semaphoreCreateInfo);
    auto subPassDependency = vk::SubpassDependency{ VK_SUBPASS_EXTERNAL,
                                                    0,
                                                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                    {},
                                                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite };
    m_renderPass = m_device.createRenderPass(vk::RenderPassCreateInfo{ {}, 1, &colorAttachment, 1, &subpass, 1, &subPassDependency });
    info.setRenderPass(m_renderPass);
    m_pipeline = m_device.createGraphicsPipeline({}, info).value;
}

void GLFWRendererVK::initProgram(std::string_view vertShader, std::string_view fragShader)
{
    auto shaderCode = ShaderVk::getSpvFromGLSL(vertShader, fragShader);
    auto vertShaderCode = shaderCode.first;
    auto vertSize = vertShaderCode.size();
    auto vertShaderCreateInfo = vk::ShaderModuleCreateInfo{ {}, vertSize * sizeof(uint32_t), vertShaderCode.data() };
    m_vertexShaderModule = m_device.createShaderModule(vertShaderCreateInfo);
    auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},
                                                                  vk::ShaderStageFlagBits::eVertex,
                                                                  m_vertexShaderModule,
                                                                  "main" };

    // fragment shader
    auto fragShaderCode = shaderCode.second;
    auto fragSize = fragShaderCode.size();
    auto fragShaderCreateInfo =
        vk::ShaderModuleCreateInfo{ {}, fragSize * sizeof(uint32_t), fragShaderCode.data() };
    m_fragmentShaderModule = m_device.createShaderModule(fragShaderCreateInfo);
    auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},
                                                                  vk::ShaderStageFlagBits::eFragment,
                                                                  m_fragmentShaderModule,
                                                                  "main" };
    m_pipelineShaderStages = { vertShaderStageInfo, fragShaderStageInfo };
}

void GLFWRendererVK::initVertexBuffer()
{
    m_vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{ {}, 0u, nullptr, 0u, nullptr };
}

void GLFWRendererVK::initCommandBuffer()
{
    uint32_t imageCount = 2;
    m_frameBuffers = std::vector<vk::Framebuffer>(imageCount);
    for (size_t i = 0; i < m_imageViews.size(); i++)
    {
        m_frameBuffers[i] = m_device.createFramebuffer(vk::FramebufferCreateInfo{
            {},
            m_renderPass,
            1,
            &(m_imageViews[i]),
            m_extent.width,
            m_extent.height,
            1 });
    }
    m_commandPool = m_device.createCommandPool({ { vk::CommandPoolCreateFlagBits::eResetCommandBuffer }, m_uniqueQueueFamilyIndices.front() });
    m_commandBuffers = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(m_frameBuffers.size())));
    m_deviceQueue = m_device.getQueue(static_cast<uint32_t>(m_uniqueQueueFamilyIndices.front()), 0);
    m_presentQueue = m_device.getQueue(static_cast<uint32_t>(m_uniqueQueueFamilyIndices.back()), 0);
}

void GLFWRendererVK::commit()
{
    for (size_t i = 0; i < m_commandBuffers.size(); i++)
    {
        auto beginInfo = vk::CommandBufferBeginInfo{};
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        m_commandBuffers[i].begin(beginInfo);
        vk::ClearValue clearValues{};
        static float red = 0.0f;
        red = red > 1.0f ? 0.0f : red + 0.005f;
        clearValues.color.setFloat32({ red, 0.0f, 0.0f, 1.0f });
        auto renderPassBeginInfo = vk::RenderPassBeginInfo{ m_renderPass, m_frameBuffers[i], vk::Rect2D{ { 0, 0 }, m_extent }, 1, &clearValues };
        m_commandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline); // 等价于 opengl的 bind program 和一设定些状态
        m_commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
        m_commandBuffers[i].draw(3, 1, 0, 0);
        m_commandBuffers[i].endRenderPass();
        m_commandBuffers[i].end();
    }
}
