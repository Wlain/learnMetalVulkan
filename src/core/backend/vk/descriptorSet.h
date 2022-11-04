//
// Created by william on 2022/11/4.
//

#ifndef LEARNMETALVULKAN_DESCRIPTORSET_H
#define LEARNMETALVULKAN_DESCRIPTORSET_H
#include "deviceVk.h"
#include "globalCommonDefine.h"

#include <map>

template <class T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;

namespace backend
{
class DescriptorSet
{
public:
    explicit DescriptorSet(Device* device);
    ~DescriptorSet();
    bool createDescriptorSetLayout(const std::vector<ShaderResource>& shaderResources);
    bool createDescriptorPool(const std::vector<vk::DescriptorPoolSize>& descriptorPoolSize, uint32_t descriptorCount);
    bool createDescriptorSets(const BindingMap<vk::DescriptorBufferInfo>& bufferInfos, const BindingMap<VkDescriptorImageInfo>& imageInfos)
    {
        auto allocInfo = vk::DescriptorSetAllocateInfo{
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_descriptorSetLayout
        };
        m_descriptorSets = m_device->handle().allocateDescriptorSets(allocInfo).front();
        m_writeDescriptorSet.clear();
        for (auto& info : bufferInfos)
        {
            auto bindingIndex = info.first;
            auto& bufferBindings = info.second;

            m_writeDescriptorSet.push_back(
                {
                    .dstBinding = bindingIndex,
                    .descriptorType = m_bindings[bindingIndex].descriptorType,
                    .pBufferInfo = bufferBindings,
                    .dstSet = m_descriptorSets,

                });
        }
        //        for (size_t i = 0; i < m_swapchainSize; i++)
        //        {
        //            auto bufferInfo = vk::DescriptorBufferInfo{
        //                .buffer = m_uniformBuffer->buffer(),
        //                .offset = 0,
        //                .range = sizeof(VertMVPMatrixUBO)
        //            };
        //            auto imageInfo = vk::DescriptorImageInfo{
        //                .sampler = m_texture->sampler(),
        //                .imageView = m_texture->imageView(),
        //                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        //            };
        //            std::array descriptorWrites = {
        //                vk::WriteDescriptorSet{
        //                    .dstSet = createDescriptorSets()[i],
        //                    .dstBinding = g_mvpMatrixUboBinding,
        //                    .dstArrayElement = 0,
        //                    .descriptorCount = 1,
        //                    .descriptorType = vk::DescriptorType::eUniformBuffer,
        //                    .pBufferInfo = &bufferInfo },
        //                vk::WriteDescriptorSet{
        //                    .dstSet = createDescriptorSets()[i],
        //                    .dstBinding = g_textureBinding,
        //                    .dstArrayElement = 0,
        //                    .descriptorCount = 1,
        //                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        //                    .pImageInfo = &imageInfo }
        //            };
        //            m_device->handle().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        //        }
        return true;
    }

private:
    vk::DescriptorType findDescriptorType(ShaderResourceType resourceType, bool dynamic);
    vk::ShaderStageFlags findShaderType(ShaderType type);

private:
    DeviceVK* m_device{ nullptr };
    std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
    // Total descriptor pools created
    vk::DescriptorPool m_descriptorPool;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    // The list of write operations for the descriptor set
    std::vector<vk::WriteDescriptorSet> m_writeDescriptorSet;
    vk::DescriptorSet m_descriptorSets;
    BindingMap<vk::DescriptorBufferInfo> m_bufferInfos;
    BindingMap<vk::DescriptorImageInfo> m_imageInfos;
};
} // namespace backend
#endif // LEARNMETALVULKAN_DESCRIPTORSET_H