//
// Created by william on 2022/11/6.
//

#include "descriptorSetGl.h"

#include "commonMacro.h"
namespace backend
{
DescriptorSetGl::DescriptorSetGl(Device* device)
{
    m_device = dynamic_cast<DeviceGl*>(device);
}

DescriptorSetGl::~DescriptorSetGl()
{
}

const std::vector<ShaderResource>& DescriptorSetGl::shaderResources() const
{
    return m_shaderResources;
}

bool DescriptorSetGl::createDescriptorSetLayout(const std::vector<ShaderResource>& shaderResources)
{
    m_shaderResources = shaderResources;
}

void DescriptorSetGl::createDescriptorSets(const std::map<int32_t, DescriptorBufferInfo>& bufferInfos, const std::map<int32_t, DescriptorImageInfo>& imageInfos)
{
    m_bufferInfos = bufferInfos;
    m_imageInfos = imageInfos;
}

std::map<int32_t, DescriptorBufferInfo>& DescriptorSetGl::bufferInfos()
{
    return m_bufferInfos;
}

std::map<int32_t, DescriptorImageInfo>& DescriptorSetGl::imageInfos()
{
    return m_imageInfos;
}

} // namespace backend