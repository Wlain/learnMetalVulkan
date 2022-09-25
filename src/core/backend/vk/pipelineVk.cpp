//
// Created by cwb on 2022/9/8.
//

#include "pipelineVk.h"
#define GLFW_INCLUDE_NONE
#include "deviceVk.h"
#include "glfwRenderer.h"
namespace backend
{
PipelineVk::PipelineVk(Device* device) :
    Pipeline(device)
{
    m_deviceVk = dynamic_cast<DeviceVK*>(device);
}

PipelineVk::~PipelineVk()
{
    auto device = m_deviceVk->handle();
    device.destroy(m_vertexShaderModule);
    device.destroy(m_fragmentShaderModule);
    device.destroy(m_pipelineLayout);
    device.destroy(m_renderPass);
    for (auto& view : m_deviceVk->swapchainImageViews())
    {
        device.destroy(view);
    }
    device.destroy(m_pipeline);
}

void PipelineVk::build()
{
    vk::GraphicsPipelineCreateInfo info;
    // set shader config
    info.setStageCount(2).setPStages(m_pipelineShaderStages.data());
    // vertex input
    info.setPVertexInputState(&m_vertexInputInfo);
    // set Assembly
    info.setPInputAssemblyState(&m_assemblyStateCreateInfo);
    // layout
    info.setLayout(m_pipelineLayout);
    // viewport and Scissor
    info.setPViewportState(&m_viewportState);
    // set rasterization
    info.setPRasterizationState(&m_rasterizer);
    // multiSample
    info.setPMultisampleState(&m_multisampling);
    // depthStencil
    info.setPDepthStencilState(nullptr);
    // color blend
    info.setPColorBlendState(&m_colorBlending);
    // renderPass
    info.setRenderPass(m_renderPass);
    m_pipeline = m_deviceVk->handle().createGraphicsPipeline({}, info).value;
}

void PipelineVk::initVertexBuffer(const VkPipelineVertexInputStateCreateInfo& info)
{
    m_vertexInputInfo = info;
}

const vk::RenderPass& PipelineVk::renderPass() const
{
    return m_renderPass;
}

vk::Pipeline PipelineVk::handle() const
{
    return m_pipeline;
}

vk::ShaderModule PipelineVk::createShaderModule(const std::vector<uint32_t>& code)
{
    auto createInfo = vk::ShaderModuleCreateInfo{
        .codeSize = code.size() * sizeof(uint32_t),
        .pCode = code.data()
    };
    return m_deviceVk->handle().createShaderModule(createInfo);
}

void PipelineVk::setProgram(std::string_view vertShader, std::string_view fragSource)
{
    auto [vertShaderCode, fragShaderCode] = PipelineVk::getSpvFromGLSL(vertShader, fragSource);
    m_vertexShaderModule = createShaderModule(vertShaderCode);
    m_fragmentShaderModule = createShaderModule(fragShaderCode);
    auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = m_vertexShaderModule,
        .pName = "main"
    };
    auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = m_fragmentShaderModule,
        .pName = "main"
    };
    m_pipelineShaderStages = { vertShaderStageInfo, fragShaderStageInfo };
}

void PipelineVk::setAssembly()
{
    m_assemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = false
    };
}

void PipelineVk::setPipelineLayout(const vk::PipelineLayout& layout)
{
    m_pipelineLayout = layout;
}

void PipelineVk::setViewport()
{
    m_viewport = vk::Viewport{
        .x = 0.0f,
        .y = static_cast<float>(m_deviceVk->height()),
        .width = static_cast<float>(m_deviceVk->width()),
        .height = -static_cast<float>(m_deviceVk->height()),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    m_scissor = vk::Rect2D{
        .offset = { 0, 0 },
        .extent = m_deviceVk->swapchainExtent()
    };
    m_viewportState = {
        .viewportCount = 1,
        .pViewports = &m_viewport,
        .scissorCount = 1,
        .pScissors = &m_scissor
    };
}

void PipelineVk::setRasterization()
{
    m_rasterizer = {
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,       // 默认值
        .frontFace = vk::FrontFace::eCounterClockwise, // 默认值
        .depthBiasEnable = false,
        .lineWidth = 1.0f
    };
}

void PipelineVk::setMultisample()
{
    m_multisampling = {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = false
    };
}

void PipelineVk::setDepthStencil()
{
}

void PipelineVk::setColorBlendAttachment()
{
    m_colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
        .blendEnable = false,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA
    };

    m_colorBlending = vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &m_colorBlendAttachment,
        .blendConstants = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }
    };
}

void PipelineVk::setRenderPass(const vk::RenderPass& renderPass)
{
    m_renderPass = m_renderPass ? renderPass : createRenderPass();
}

vk::RenderPass PipelineVk::createRenderPass()
{
    vk::RenderPass renderPass;
    auto colorAttachment = vk::AttachmentDescription{
        .format = m_deviceVk->swapchainImageFormat(),
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
        .colorAttachmentCount = 0,
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
    renderPass = m_deviceVk->handle().createRenderPass(renderPassInfo);
    return renderPass;
}
} // namespace backend