//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERVK_H
#define LEARNMETALVULKAN_SHADERVK_H
#include "pipeline.h"
#include "vkCommonDefine.h"
namespace backend
{
class DeviceVk;

class PipelineVk : public Pipeline
{
public:
    explicit PipelineVk(Device* device);
    ~PipelineVk() override;
    void setProgram(std::string_view vertShader, std::string_view fragSource) override;
    void setPipelineLayout(vk::PipelineLayout layout);
    void setViewport();
    void setRasterization();
    void setMultisample();
    void setAttributeDescription(const std::vector<AttributeDescription>& attributeDescriptions) override;
    void setDepthStencil(vk::PipelineDepthStencilStateCreateInfo info);
    void setColorBlendAttachment();
    void setRenderPass();
    void build() override;
    [[nodiscard]] vk::Pipeline handle() const;
    [[nodiscard]] vk::Pipeline& handle();
    void setTopology(Topology topology) override;

private:
    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code);
    vk::PipelineShaderStageCreateInfo getShaderCreateInfo(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits stage);

private:
    DeviceVk* m_deviceVk{ nullptr };
    vk::RenderPass m_renderPass;
    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    std::vector<vk::PipelineShaderStageCreateInfo> m_pipelineShaderStages;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly;
    vk::Viewport m_viewport;
    vk::Rect2D m_scissor;
    vk::PipelineViewportStateCreateInfo m_viewportState;
    vk::PipelineRasterizationStateCreateInfo m_rasterizer;
    vk::PipelineMultisampleStateCreateInfo m_multisampling;
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilState;
    vk::PipelineColorBlendAttachmentState m_colorBlendAttachment;
    vk::PipelineColorBlendStateCreateInfo m_colorBlending;
    vk::VertexInputBindingDescription m_bindingDescription;
    std::vector<vk::VertexInputAttributeDescription> m_inputAttributeDescriptions{};
    vk::PrimitiveTopology m_topology{};
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERVK_H
