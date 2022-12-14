//
// Created by cwb on 2022/9/26.
//

#include "bufferVk.h"

#include "commonMacro.h"
#include "deviceVk.h"

namespace backend
{
BufferVk::BufferVk(Device* device) :
    Buffer(device)
{
    m_deviceVk = static_cast<DeviceVk*>(device);
}

BufferVk::~BufferVk()
{
    auto device = m_deviceVk->handle();
    device.destroyBuffer(m_buffer);
    device.freeMemory(m_deviceMemory);
}

std::pair<vk::Buffer, vk::DeviceMemory> BufferVk::createBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    auto device = m_deviceVk->handle();
    auto bufferInfo = vk::BufferCreateInfo{
        .size = bufferSize,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    auto buffer = device.createBuffer(bufferInfo);
    auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
    auto allocInfo = vk::MemoryAllocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = m_deviceVk->findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };
    auto bufferMemory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(buffer, bufferMemory, 0);
    return { buffer, bufferMemory };
}

vk::BufferUsageFlags BufferVk::getUsageFlag(BufferUsage usage)
{
    vk::BufferUsageFlags usageFlags;
    switch (usage)
    {
    case BufferUsage::StaticDraw:
        usageFlags = vk::BufferUsageFlagBits::eTransferDst;
        break;
    case BufferUsage::DynamicDraw:
        ASSERT(0);
        break;
    case BufferUsage::StreamDraw:
        ASSERT(0);
        break;
    }
    return usageFlags;
}

vk::BufferUsageFlags BufferVk::getBufferType(Buffer::BufferType type)
{
    vk::BufferUsageFlags usageFlags;
    switch (type)
    {
    case BufferType::VertexBuffer:
        usageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
        break;
    case BufferType::IndexBuffer:
        usageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
        break;
    case BufferType::UniformBuffer:
        usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
        break;
    }
    return usageFlags;
}

void BufferVk::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    auto commandBuffer = m_deviceVk->beginSingleTimeCommands();
    auto copyRegion = vk::BufferCopy{ .size = size };
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    m_deviceVk->endSingleTimeCommands(commandBuffer);
}

void BufferVk::create(size_t bufferSize, void* data, BufferUsage usage, BufferType type)
{
    auto device = m_deviceVk->handle();
    m_bufferSize = bufferSize;
    //    auto usageFlags = getUsageFlag(usage);
    auto bufferTypeFlags = getBufferType(type);
    auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    // copy the vertex and color data into that device memory
    auto* pData = (device.mapMemory(stagingBufferMemory, {}, bufferSize, {}));
    memcpy(pData, data, static_cast<std::size_t>(bufferSize));
    device.unmapMemory(stagingBufferMemory);
    std::tie(m_buffer, m_deviceMemory) = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | bufferTypeFlags, vk::MemoryPropertyFlagBits::eHostVisible);
    copyBuffer(stagingBuffer, m_buffer, bufferSize);
    device.destroyBuffer(stagingBuffer);
    device.freeMemory(stagingBufferMemory);
}

void BufferVk::update(void* data, size_t size, size_t offset)
{
    auto device = m_deviceVk->handle();
    void* target{};
    void* pData = device.mapMemory(m_deviceMemory, {}, size, {});
    target = (uint8_t*)pData + offset;
    memcpy(target, data, size);
    device.unmapMemory(m_deviceMemory);
}

const vk::Buffer& BufferVk::buffer() const
{
    return m_buffer;
}

const vk::DeviceMemory& BufferVk::deviceMemory() const
{
    return m_deviceMemory;
}

size_t BufferVk::bufferSize() const
{
    return m_bufferSize;
}
} // namespace backend