//
// Created by william on 2022/9/27.
//

#include "bufferMtl.h"

#include "mtlCommonDefine.h"
namespace backend
{
BufferMTL::BufferMTL(Device* device) :
    Buffer(device)
{
    m_deviceMtl = static_cast<DeviceMtl*>(device);
}

BufferMTL::~BufferMTL()
{
    m_buffer->release();
}

void BufferMTL::create(size_t bufferSize, void* data, Buffer::BufferUsage usage, Buffer::BufferType type)
{
    // static:使用MTL::ResourceCPUCacheModeWriteCombined会获得性能提升
    // other:MTLResourceStorageModeShared
    auto usageFlag = MTL::ResourceStorageModeShared;
    m_buffer = m_deviceMtl->gpu()->newBuffer(data, bufferSize, usageFlag);
}

void BufferMTL::update(void* data, size_t size, size_t offset)
{
    Buffer::update(data, size, offset);
}

MTL::Buffer* BufferMTL::buffer() const
{
    return m_buffer;
}
} // namespace backend
