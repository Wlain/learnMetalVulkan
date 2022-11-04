//
// Created by william on 2022/11/4.
//

#include "descriptorSet.h"
namespace backend
{
DescriptorSet::DescriptorSet(Device* device)
{
    m_device = static_cast<DeviceVK*>(device);
}

DescriptorSet::~DescriptorSet()
{
}
} // namespace backend