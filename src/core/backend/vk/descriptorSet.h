//
// Created by william on 2022/11/4.
//

#ifndef LEARNMETALVULKAN_DESCRIPTORSET_H
#define LEARNMETALVULKAN_DESCRIPTORSET_H
#include "deviceVk.h"

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

private:
    DeviceVK* m_device{ nullptr };
    std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
    BindingMap<VkDescriptorBufferInfo> m_bufferInfos;
    BindingMap<VkDescriptorImageInfo> m_imageInfos;
    // Total descriptor pools created
    std::vector<vk::DescriptorPool> m_pools;
    vk::DescriptorSetLayout m_layout;
};
} // namespace backend
#endif // LEARNMETALVULKAN_DESCRIPTORSET_H
