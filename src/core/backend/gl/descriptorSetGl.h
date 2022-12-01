//
// Created by william on 2022/11/6.
//

#ifndef LEARNMETALVULKAN_DESCRIPTORSETGL_H
#define LEARNMETALVULKAN_DESCRIPTORSETGL_H
#include "deviceGl.h"
#include "globalCommonDefine.h"

#include <map>
#include <vector>
namespace backend
{
struct DescriptorBufferInfo
{
    uint32_t bufferType;
    uint32_t buffer{};
    uint32_t offset{};
    uint32_t range{};
};

struct DescriptorImageInfo
{
    std::string name;
    uint32_t target;
    uint32_t texture;
};

class DescriptorSetGl
{
public:
    explicit DescriptorSetGl(Device* device);
    ~DescriptorSetGl();
    bool createDescriptorSetLayout(const std::vector<ShaderResource>& shaderResources);
    void createDescriptorSets(const std::map<uint32_t, DescriptorBufferInfo>& bufferInfos, const std::map<uint32_t, DescriptorImageInfo>& imageInfos);
    std::map<uint32_t, DescriptorBufferInfo>& bufferInfos();
    std::map<uint32_t, DescriptorImageInfo>& imageInfos();
    const std::vector<ShaderResource>& shaderResources() const;

private:
    DeviceGl* m_device{ nullptr };
    std::vector<ShaderResource> m_shaderResources;
    std::map<uint32_t, DescriptorBufferInfo> m_bufferInfos;
    std::map<uint32_t, DescriptorImageInfo> m_imageInfos;
};
} // namespace backend

#endif // LEARNMETALVULKAN_DESCRIPTORSETGL_H
