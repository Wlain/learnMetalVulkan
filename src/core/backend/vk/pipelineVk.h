//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERVK_H
#define LEARNMETALVULKAN_SHADERVK_H
#include "pipeline.h"
#define VULKAN_HPP_NO_CONSTRUCTORS // 从 vulkan.hpp 中删除所有结构和联合构造函数
#include <vulkan/vulkan.hpp>
namespace backend
{
class DeviceVK;

class PipelineVk : public Pipeline
{
public:
    explicit PipelineVk(Device* device);
    ~PipelineVk() override;
    void setProgram(std::string_view vertShader, std::string_view fragSource) override;
    void initVertexBuffer(const VkPipelineVertexInputStateCreateInfo& info);
    void setAssembly();
    void setPipelineLayout(const vk::PipelineLayout& layout);
    void setViewport();
    void setRasterization();
    void setMultisample();
    void setDepthStencil();
    void setColorBlendAttachment();
    void setRenderPass(const vk::RenderPass& renderPass = nullptr);
    void build() override;
    const vk::RenderPass& renderPass() const;
    vk::Pipeline handle() const;

private:
    vk::RenderPass createRenderPass();
    vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code);

private:
    DeviceVK* m_deviceVk{ nullptr };
    vk::RenderPass m_renderPass;
    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    std::vector<vk::PipelineShaderStageCreateInfo> m_pipelineShaderStages;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo m_assemblyStateCreateInfo;
    vk::Viewport m_viewport;
    vk::Rect2D m_scissor;
    vk::PipelineViewportStateCreateInfo m_viewportState;
    vk::PipelineRasterizationStateCreateInfo m_rasterizer;
    vk::PipelineMultisampleStateCreateInfo m_multisampling;
    vk::PipelineColorBlendAttachmentState m_colorBlendAttachment;
    vk::PipelineColorBlendStateCreateInfo m_colorBlending;
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERVK_H
