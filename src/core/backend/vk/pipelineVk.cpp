//
// Created by cwb on 2022/9/8.
//

#include "pipelineVk.h"
#include "commonMacro.h"
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
    device.destroy(m_pipelineLayout);
    device.destroy(m_pipeline);
}

void PipelineVk::build()
{
    auto pipelineInfo = vk::GraphicsPipelineCreateInfo{
        .stageCount = 2,
        .pStages = m_pipelineShaderStages.data(),
        .pVertexInputState = &m_vertexInputInfo,
        .pInputAssemblyState = &m_inputAssembly,
        .pViewportState = &m_viewportState,
        .pRasterizationState = &m_rasterizer,
        .pMultisampleState = &m_multisampling,
        .pDepthStencilState = &m_depthStencilState,
        .pColorBlendState = &m_colorBlending,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0
    };
    m_pipeline = m_deviceVk->handle().createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo).value;
    m_deviceVk->handle().destroyShaderModule(m_vertexShaderModule);
    m_deviceVk->handle().destroyShaderModule(m_fragmentShaderModule);
}

void PipelineVk::initVertexBuffer(const VkPipelineVertexInputStateCreateInfo& info)
{
    m_vertexInputInfo = info;
}

vk::Pipeline PipelineVk::handle() const
{
    return m_pipeline;
}

vk::Pipeline& PipelineVk::handle()
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
    auto vertShaderStageInfo = getShaderCreateInfo(m_vertexShaderModule, vk::ShaderStageFlagBits::eVertex);
    auto fragShaderStageInfo = getShaderCreateInfo(m_fragmentShaderModule, vk::ShaderStageFlagBits::eFragment);
    m_pipelineShaderStages = { vertShaderStageInfo, fragShaderStageInfo };
}

void PipelineVk::setPipelineLayout(vk::PipelineLayout layout)
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

void PipelineVk::setDepthStencil(vk::PipelineDepthStencilStateCreateInfo info)
{
    m_depthStencilState = info;
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

void PipelineVk::setRenderPass()
{
    m_renderPass = m_deviceVk->renderPass();
}

void PipelineVk::setTopology(Topology topology)
{
    switch (topology)
    {
    case Topology::Points:
        m_topology = vk::PrimitiveTopology::ePointList;
        break;
    case Topology::LineStrip:
        m_topology = vk::PrimitiveTopology::eLineStrip;
        break;
    case Topology::Lines:
        m_topology = vk::PrimitiveTopology::eLineList;
        break;
    case Topology::Triangles:
        m_topology = vk::PrimitiveTopology::eTriangleList;
        break;
    case Topology::TriangleStrip:
        m_topology = vk::PrimitiveTopology::eTriangleStrip;
        break;
    default:
        break;
    }
    m_inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
        .topology = m_topology,
        .primitiveRestartEnable = false
    };
}

vk::PipelineShaderStageCreateInfo PipelineVk::getShaderCreateInfo(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits stage)
{
    vk::PipelineShaderStageCreateInfo shaderStage = {
        .stage = stage,
        .module = shaderModule,
        .pName = "main"
    };
    ASSERT(shaderStage.module);
    return shaderStage;
}
} // namespace backend