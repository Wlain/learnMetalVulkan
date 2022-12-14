//
// Created by cwb on 2022/9/26.
//

#ifndef LEARNMETALVULKAN_BufferVK_H
#define LEARNMETALVULKAN_BufferVK_H
#include "buffer.h"
#include "deviceVk.h"

#include "vkCommonDefine.h"
namespace backend
{
class BufferVk : public Buffer
{
public:
    explicit BufferVk(Device* device);
    ~BufferVk() override;
    void create(size_t size, void* data, BufferUsage usage, BufferType type) override;
    void update(void* data, size_t size, size_t offset) override;
    const vk::Buffer& buffer() const;
    const vk::DeviceMemory& deviceMemory() const;
    size_t bufferSize() const;

protected:
    vk::BufferUsageFlags getBufferType(Buffer::BufferType type);
    std::pair<vk::Buffer, vk::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

private:
    vk::BufferUsageFlags getUsageFlag(BufferUsage usage);

protected:
    DeviceVk* m_deviceVk{ nullptr };
    vk::Buffer m_buffer;
    vk::DeviceMemory m_deviceMemory;
    size_t m_bufferSize;
};
} // namespace backend

#endif // LEARNMETALVULKAN_BufferVK_H
