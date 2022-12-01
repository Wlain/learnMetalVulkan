//
// Created by william on 2022/11/4.
//

#ifndef LEARNMETALVULKAN_DESCRIPTORSETVK_H
#define LEARNMETALVULKAN_DESCRIPTORSETVK_H
#include "deviceVk.h"
#include "globalCommonDefine.h"

#include <map>
namespace backend
{
class DescriptorSetVk
{
public:
    explicit DescriptorSetVk(Device* device);
    ~DescriptorSetVk();
    bool createDescriptorSetLayout(const std::vector<ShaderResource>& shaderResources);
    bool createDescriptorPool(const std::vector<vk::DescriptorPoolSize>& descriptorPoolSize, uint32_t descriptorCount);
    bool createDescriptorSets(const std::map<uint32_t, vk::DescriptorBufferInfo>& bufferInfos, const std::map<uint32_t, vk::DescriptorImageInfo>& imageInfos);
    const vk::DescriptorSet& handle() const;
    const vk::DescriptorSetLayout& layout() const;

private:
    static vk::DescriptorType findDescriptorType(ShaderResourceType resourceType, bool dynamic);
    static vk::ShaderStageFlags findShaderType(ShaderType type);

private:
    DeviceVK* m_device{ nullptr };
    std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
    std::map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindingsLookup;
    // Total descriptor pools created
    vk::DescriptorPool m_descriptorPool;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    // The list of write operations for the descriptor set
    std::vector<vk::WriteDescriptorSet> m_writeDescriptorSet;
    vk::DescriptorSet m_descriptorSets;
    std::map<uint32_t, vk::DescriptorBufferInfo> m_bufferInfos;
    std::map<uint32_t, vk::DescriptorImageInfo> m_imageInfos;
};
} // namespace backend
#endif // LEARNMETALVULKAN_DESCRIPTORSETVK_H