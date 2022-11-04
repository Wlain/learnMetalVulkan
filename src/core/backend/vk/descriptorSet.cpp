//
// Created by william on 2022/11/4.
//

#include "descriptorSet.h"

#include "commonMacro.h"
namespace backend
{
DescriptorSet::DescriptorSet(Device* device)
{
    ASSERT(device);
    m_device = static_cast<DeviceVK*>(device);
}

DescriptorSet::~DescriptorSet()
{
}

bool DescriptorSet::createDescriptorPool(const std::vector<vk::DescriptorPoolSize>& descriptorPoolSize, uint32_t descriptorCount)
{
    auto poolInfo = vk::DescriptorPoolCreateInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = descriptorCount,
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size()),
        .pPoolSizes = descriptorPoolSize.data()
    };
    m_descriptorPool = m_device->handle().createDescriptorPool(poolInfo);
    return true;
}

vk::DescriptorType DescriptorSet::findDescriptorType(ShaderResourceType resourceType, bool dynamic)
{
    vk::DescriptorType result{};
    switch (resourceType)
    {
    case ShaderResourceType::InputAttachment:
        result = vk::DescriptorType::eInputAttachment;
        break;
    case ShaderResourceType::Image:
        result = vk::DescriptorType::eSampledImage;
        break;
    case ShaderResourceType::ImageSampler:
        result = vk::DescriptorType::eCombinedImageSampler;
        break;
    case ShaderResourceType::ImageStorage:
        result = vk::DescriptorType::eStorageImage;
        break;
    case ShaderResourceType::Sampler:
        result = vk::DescriptorType::eCombinedImageSampler;
        break;
    case ShaderResourceType::BufferUniform:
        if (dynamic)
            result = vk::DescriptorType::eUniformBufferDynamic;
        else
            result = vk::DescriptorType::eUniformBuffer;
        break;
    case ShaderResourceType::BufferStorage:
        if (dynamic)
            result = vk::DescriptorType::eStorageBufferDynamic;
        else
            result = vk::DescriptorType::eStorageBuffer;
        break;
    case ShaderResourceType::All:
    default:
        break;
    }
    return result;
}

vk::ShaderStageFlags DescriptorSet::findShaderType(ShaderType type)
{
    vk::ShaderStageFlags result{};
    switch (type)
    {
    case ShaderType::Vertex:
        result = vk::ShaderStageFlagBits::eVertex;
        break;
    case ShaderType::Fragment:
        result = vk::ShaderStageFlagBits::eFragment;
        break;
    }
    return result;
}

bool DescriptorSet::createDescriptorSetLayout(const std::vector<ShaderResource>& shaderResources)
{
    m_bindings.clear();
    for (const auto& resource : shaderResources)
    {
        auto descriptorType = findDescriptorType(resource.type, resource.mode == ShaderResourceMode::Dynamic);
        m_bindings.emplace_back(vk::DescriptorSetLayoutBinding{
            .binding = resource.binding,
            .descriptorType = descriptorType,
            .descriptorCount = resource.arraySize,
            .stageFlags = findShaderType(resource.stages),
            .pImmutableSamplers = nullptr // optional (only relevant to Image Sampling;
        });
    }
    auto layoutInfo = vk::DescriptorSetLayoutCreateInfo{
        .bindingCount = static_cast<uint32_t>(m_bindings.size()),
        .pBindings = m_bindings.data()
    };
    m_descriptorSetLayout = m_device->handle().createDescriptorSetLayout(layoutInfo);
    return true;
}

} // namespace backend