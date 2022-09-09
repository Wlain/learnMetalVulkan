//
// Created by cwb on 2022/9/8.
//

#include "pipelineVk.h"

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include "deviceVk.h"
#include "glfwRenderer.h"
namespace backend
{
PipelineVk::PipelineVk(Device* handle) :
    Pipeline(handle)
{
    m_deviceVk = dynamic_cast<DeviceVK*>(handle);
}

PipelineVk::~PipelineVk()
{
    auto device = m_deviceVk->handle();
    device.destroy(m_vertexShaderModule);
    device.destroy(m_fragmentShaderModule);
    device.destroy(m_pipelineLayout);
    device.destroy(m_renderPass);
    for (auto& view : m_deviceVk->imageViews())
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

void PipelineVk::initVertexBuffer()
{
    m_vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{ {}, 0u, nullptr, 0u, nullptr };
}

const vk::RenderPass& PipelineVk::renderPass() const
{
    return m_renderPass;
}

vk::Pipeline PipelineVk::handle() const
{
    return m_pipeline;
}

void PipelineVk::setProgram(std::string_view vertShader, std::string_view fragSource)
{
    auto shaderCode = PipelineVk::getSpvFromGLSL(vertShader, fragSource);
    auto vertShaderCode = shaderCode.first;
    auto vertSize = vertShaderCode.size();
    auto vertShaderCreateInfo = vk::ShaderModuleCreateInfo{ {}, vertSize * sizeof(uint32_t), vertShaderCode.data() };
    m_vertexShaderModule = m_deviceVk->handle().createShaderModule(vertShaderCreateInfo);
    auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},
                                                                  vk::ShaderStageFlagBits::eVertex,
                                                                  m_vertexShaderModule,
                                                                  "main" };

    // fragment shader
    auto fragShaderCode = shaderCode.second;
    auto fragSize = fragShaderCode.size();
    auto fragShaderCreateInfo =
        vk::ShaderModuleCreateInfo{ {}, fragSize * sizeof(uint32_t), fragShaderCode.data() };
    m_fragmentShaderModule = m_deviceVk->handle().createShaderModule(fragShaderCreateInfo);
    auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},
                                                                  vk::ShaderStageFlagBits::eFragment,
                                                                  m_fragmentShaderModule,
                                                                  "main" };
    m_pipelineShaderStages = { vertShaderStageInfo, fragShaderStageInfo };
}

void PipelineVk::setAssembly()
{
    m_assemblyStateCreateInfo = { {}, vk::PrimitiveTopology::eTriangleList, false };
}

void PipelineVk::setPipelineLayout()
{
    m_pipelineLayout = m_deviceVk->handle().createPipelineLayout({}, nullptr);
}

void PipelineVk::setViewport()
{
    m_viewport = vk::Viewport{ 0.0f, 0.0f, static_cast<float>(m_deviceVk->width()), static_cast<float>(m_deviceVk->height()), 0.0f, 1.0f };
    m_scissor = vk::Rect2D{ { 0, 0 }, {} };
    m_viewportState = { {}, 1, &m_viewport, 1, &m_scissor };
}

void PipelineVk::setRasterization()
{
    m_rasterizer = { {}, /*depthClamp*/ false,
                     /*rasterizeDiscard*/ false,
                     vk::PolygonMode::eFill,
                     {},
                     /*frontFace*/ vk::FrontFace::eCounterClockwise,
                     {},
                     {},
                     {},
                     {},
                     1.0f };
}

void PipelineVk::setMultisample()
{
    m_multisampling = { {}, vk::SampleCountFlagBits::e1, false, 1.0 };
}

void PipelineVk::setDepthStencil()
{
}

void PipelineVk::setColorBlendAttachment()
{
    m_colorBlendAttachment = { {}, /*srcCol*/ vk::BlendFactor::eOne,
                               /*dstCol*/ vk::BlendFactor::eZero,
                               /*colBlend*/ vk::BlendOp::eAdd,
                               /*srcAlpha*/ vk::BlendFactor::eOne,
                               /*dstAlpha*/ vk::BlendFactor::eZero,
                               /*alphaBlend*/ vk::BlendOp::eAdd,
                               vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };

    m_colorBlending = { {},
                        /*logicOpEnable=*/false,
                        vk::LogicOp::eCopy,
                        /*attachmentCount=*/1,
                        /*colourAttachments=*/&m_colorBlendAttachment };
}

void PipelineVk::setRenderPass()
{
    auto format = vk::Format::eB8G8R8A8Unorm;
    auto colorAttachment = vk::AttachmentDescription{ {}, format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, {}, vk::ImageLayout::ePresentSrcKHR };
    auto colourAttachmentRef = vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };
    auto subpass = vk::SubpassDescription{ {},
                                           vk::PipelineBindPoint::eGraphics,
                                           /*inAttachmentCount*/ 0,
                                           nullptr,
                                           1,
                                           &colourAttachmentRef };
    auto subPassDependency = vk::SubpassDependency{ VK_SUBPASS_EXTERNAL,
                                                    0,
                                                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                    {},
                                                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite };
    m_renderPass = m_deviceVk->handle().createRenderPass(vk::RenderPassCreateInfo{ {}, 1, &colorAttachment, 1, &subpass, 1, &subPassDependency });
}
} // namespace backend