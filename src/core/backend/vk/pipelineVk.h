//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERVK_H
#define LEARNMETALVULKAN_SHADERVK_H
#include "pipeline.h"

#include <vulkan/vulkan.hpp>
namespace backend
{
class DeviceVK;
class PipelineVk : public Pipeline
{
public:
    explicit PipelineVk(Device* handle);
    ~PipelineVk() override = default;
    void initialize() override;

private:
    void initProgram();

private:
    DeviceVK* m_deviceVk{ nullptr };
    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> m_pipelineShaderStages;
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERVK_H
