//
// Created by cwb on 2022/9/8.
//

#include "pipelineVk.h"

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include "deviceVk.h"
#include "glfwRenderer.h"
#include <iostream>


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

static const std::vector<uint32_t> fragShaderCodeTemp = {
    0x07230203, 0x00010000, 0x00080007, 0x00000014, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00000011, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x00000190, 0x00090004, 0x415f4c47, 0x735f4252,
    0x72617065, 0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00090004, 0x415f4c47,
    0x735f4252, 0x69646168, 0x6c5f676e, 0x75676e61, 0x5f656761, 0x70303234, 0x006b6361, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x61724675, 0x6c6f4367, 0x0000726f,
    0x00030005, 0x0000000d, 0x00786574, 0x00050005, 0x00000011, 0x63786574, 0x64726f6f, 0x00000000,
    0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x00000022, 0x00000000,
    0x00040047, 0x0000000d, 0x00000021, 0x00000000, 0x00040047, 0x00000011, 0x0000001e, 0x00000000,
    0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020,
    0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007,
    0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00090019, 0x0000000a, 0x00000006, 0x00000001,
    0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x0000000b, 0x0000000a,
    0x00040020, 0x0000000c, 0x00000000, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000000,
    0x00040017, 0x0000000f, 0x00000006, 0x00000002, 0x00040020, 0x00000010, 0x00000001, 0x0000000f,
    0x0004003b, 0x00000010, 0x00000011, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000b, 0x0000000e, 0x0000000d, 0x0004003d,
    0x0000000f, 0x00000012, 0x00000011, 0x00050057, 0x00000007, 0x00000013, 0x0000000e, 0x00000012,
    0x0003003e, 0x00000009, 0x00000013, 0x000100fd, 0x00010038
};

static const std::vector<uint32_t> vertShaderCodeTemp = {
    0x07230203, 0x00010000, 0x00080007, 0x00000018, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0009000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000010,
    0x00000014, 0x00030003, 0x00000002, 0x00000190, 0x00090004, 0x415f4c47, 0x735f4252, 0x72617065,
    0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00090004, 0x415f4c47, 0x735f4252,
    0x69646168, 0x6c5f676e, 0x75676e61, 0x5f656761, 0x70303234, 0x006b6361, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x63786574, 0x64726f6f, 0x00000000, 0x00040005,
    0x0000000b, 0x72747461, 0x00000000, 0x00060005, 0x0000000e, 0x505f6c67, 0x65567265, 0x78657472,
    0x00000000, 0x00060006, 0x0000000e, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005,
    0x00000010, 0x00000000, 0x00030005, 0x00000014, 0x00736f70, 0x00040047, 0x00000009, 0x0000001e,
    0x00000000, 0x00040047, 0x0000000b, 0x0000001e, 0x00000001, 0x00050048, 0x0000000e, 0x00000000,
    0x0000000b, 0x00000000, 0x00030047, 0x0000000e, 0x00000002, 0x00040047, 0x00000014, 0x0000001e,
    0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
    0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000002, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a, 0x00000001,
    0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00040017, 0x0000000d, 0x00000006,
    0x00000004, 0x0003001e, 0x0000000e, 0x0000000d, 0x00040020, 0x0000000f, 0x00000003, 0x0000000e,
    0x0004003b, 0x0000000f, 0x00000010, 0x00000003, 0x00040015, 0x00000011, 0x00000020, 0x00000001,
    0x0004002b, 0x00000011, 0x00000012, 0x00000000, 0x00040020, 0x00000013, 0x00000001, 0x0000000d,
    0x0004003b, 0x00000013, 0x00000014, 0x00000001, 0x00040020, 0x00000016, 0x00000003, 0x0000000d,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
    0x00000007, 0x0000000c, 0x0000000b, 0x0003003e, 0x00000009, 0x0000000c, 0x0004003d, 0x0000000d,
    0x00000015, 0x00000014, 0x00050041, 0x00000016, 0x00000017, 0x00000010, 0x00000012, 0x0003003e,
    0x00000017, 0x00000015, 0x000100fd, 0x00010038
};

void PipelineVk::setProgram(std::string_view vertShader, std::string_view fragSource)
{
    auto shaderCode = PipelineVk::getSpvFromGLSL(vertShader, fragSource);
    auto vtShader = PipelineVk::getGlShaderFromSpv(vertShaderCodeTemp);
    std::cout << "vtShader：\n" << vtShader.c_str();
    auto fgShader = PipelineVk::getGlShaderFromSpv(fragShaderCodeTemp);
    std::cout << "fgShader：\n" << fgShader.c_str();
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