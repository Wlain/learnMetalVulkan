//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVk.h"

#include "pipelineVk.h"
namespace backend
{
GLFWRendererVK::GLFWRendererVK(Device* handle) :
    GLFWRenderer(handle)
{
    m_handleVk = dynamic_cast<DeviceVK*>(handle);
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
    initPhysicalDevice();
    initDevice();
    createSwapChain();
}

void GLFWRendererVK::initPhysicalDevice()
{
    m_gpu = m_handleVk->gpu();
}

void GLFWRendererVK::initDevice()
{
    m_device = m_handleVk->device();
}

void GLFWRendererVK::createSwapChain()
{
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

void GLFWRendererVK::setPipeline(const Pipeline& pipeline)
{
}

void GLFWRendererVK::initProgram(std::string_view vertShader, std::string_view fragShader)
{
}

void GLFWRendererVK::initInstance()
{
    m_instance = m_handleVk->instance();
}
} // namespace backend