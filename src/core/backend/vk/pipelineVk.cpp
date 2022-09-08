//
// Created by cwb on 2022/9/8.
//

#include "pipelineVk.h"

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include "glfwRenderer.h"
#include "deviceVk.h"
namespace backend
{
PipelineVk::PipelineVk(Device* handle) :
    Pipeline(handle)
{
    m_deviceVk = dynamic_cast<DeviceVK*>(handle);
}

void PipelineVk::initialize()
{
    initProgram();
}

void PipelineVk::initProgram()
{
    auto shaderCode = PipelineVk::getSpvFromGLSL(m_vertShader, m_fragShader);
    auto vertShaderCode = shaderCode.first;
    auto vertSize = vertShaderCode.size();
    auto vertShaderCreateInfo = vk::ShaderModuleCreateInfo{ {}, vertSize * sizeof(uint32_t), vertShaderCode.data() };
    m_vertexShaderModule = m_deviceVk->device().createShaderModule(vertShaderCreateInfo);
    auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},
                                                                  vk::ShaderStageFlagBits::eVertex,
                                                                  m_vertexShaderModule,
                                                                  "main" };

    // fragment shader
    auto fragShaderCode = shaderCode.second;
    auto fragSize = fragShaderCode.size();
    auto fragShaderCreateInfo =
        vk::ShaderModuleCreateInfo{ {}, fragSize * sizeof(uint32_t), fragShaderCode.data() };
    m_fragmentShaderModule = m_deviceVk->device().createShaderModule(fragShaderCreateInfo);
    auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{ {},
                                                                  vk::ShaderStageFlagBits::eFragment,
                                                                  m_fragmentShaderModule,
                                                                  "main" };
    m_pipelineShaderStages = { vertShaderStageInfo, fragShaderStageInfo };
}
} // namespace backend