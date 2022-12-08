//
// Created by william on 2022/11/4.
//

#include "descriptorSetVk.h"

#include "commonMacro.h"
namespace backend
{
DescriptorSetVk::DescriptorSetVk(Device* device)
{
    ASSERT(device);
    m_device = dynamic_cast<DeviceVk*>(device);
}

DescriptorSetVk::~DescriptorSetVk()
{
    m_device->handle().destroy(m_descriptorSetLayout);
    m_device->handle().freeDescriptorSets(m_descriptorPool, m_descriptorSets);
    m_device->handle().destroy(m_descriptorPool);
}

bool DescriptorSetVk::createDescriptorPool(const std::vector<vk::DescriptorPoolSize>& descriptorPoolSize, uint32_t descriptorCount)
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

vk::DescriptorType DescriptorSetVk::findDescriptorType(ShaderResourceType resourceType, bool dynamic)
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

vk::ShaderStageFlags DescriptorSetVk::findShaderType(ShaderType type)
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
    case ShaderType::Unknown:
        break;
    }
    return result;
}

bool DescriptorSetVk::createDescriptorSetLayout(const std::vector<ShaderResource>& shaderResources)
{
    m_bindings.clear();
    for (const auto& resource : shaderResources)
    {
        auto descriptorType = findDescriptorType(resource.type, resource.mode == ShaderResourceMode::Dynamic);
        auto layoutBinding = vk::DescriptorSetLayoutBinding{
            .binding = resource.binding,
            .descriptorType = descriptorType,
            .descriptorCount = resource.arraySize,
            .stageFlags = findShaderType(resource.stages),
            .pImmutableSamplers = nullptr // optional (only relevant to Image Sampling;
        };
        m_bindingsLookup.emplace(resource.binding, layoutBinding);
        m_bindings.emplace_back(layoutBinding);
    }
    auto layoutInfo = vk::DescriptorSetLayoutCreateInfo{
        .bindingCount = static_cast<uint32_t>(m_bindings.size()),
        .pBindings = m_bindings.data()
    };
    m_descriptorSetLayout = m_device->handle().createDescriptorSetLayout(layoutInfo);
    return true;
}

bool DescriptorSetVk::createDescriptorSets(const std::map<uint32_t, vk::DescriptorBufferInfo>& bufferInfos, const std::map<uint32_t, vk::DescriptorImageInfo>& imageInfos)
{
    m_bufferInfos = bufferInfos;
    m_imageInfos = imageInfos;
    auto allocInfo = vk::DescriptorSetAllocateInfo{
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptorSetLayout
    };
    m_descriptorSets = m_device->handle().allocateDescriptorSets(allocInfo).front();
    m_writeDescriptorSet.clear();
    for (const auto& info : m_bufferInfos)
    {
        auto bindingIndex = info.first;
        auto& bufferBindings = info.second;
        auto it = m_bindingsLookup.find(bindingIndex);
        if (it != m_bindingsLookup.end())
        {
            const auto& binding = it->second;
            m_writeDescriptorSet.push_back(
                vk::WriteDescriptorSet{
                    .dstBinding = bindingIndex,
                    .descriptorType = binding.descriptorType,
                    .pBufferInfo = &bufferBindings,
                    .dstSet = m_descriptorSets,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                });
        }
    }
    for (const auto& info : m_imageInfos)
    {
        auto bindingIndex = info.first;
        const auto& bufferBindings = info.second;
        auto it = m_bindingsLookup.find(bindingIndex);
        if (it != m_bindingsLookup.end())
        {
            auto& binding = it->second;
            m_writeDescriptorSet.push_back(
                vk::WriteDescriptorSet{
                    .dstBinding = bindingIndex,
                    .descriptorType = binding.descriptorType,
                    .pImageInfo = &bufferBindings,
                    .dstSet = m_descriptorSets,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                });
        }
    }
    m_device->handle().updateDescriptorSets(static_cast<uint32_t>(m_writeDescriptorSet.size()), m_writeDescriptorSet.data(), 0, nullptr);
    return true;
}

const vk::DescriptorSet& DescriptorSetVk::handle() const
{
    return m_descriptorSets;
}

const vk::DescriptorSetLayout& DescriptorSetVk::layout() const
{
    return m_descriptorSetLayout;
}

} // namespace backend