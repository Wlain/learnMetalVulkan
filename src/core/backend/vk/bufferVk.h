//
// Created by cwb on 2022/9/26.
//

#ifndef LEARNMETALVULKAN_BufferVK_H
#define LEARNMETALVULKAN_BufferVK_H
#include "buffer.h"
#include "deviceVk.h"

#define VULKAN_HPP_NO_CONSTRUCTORS // 从 vulkan.hpp 中删除所有结构和联合构造函数
#include <vulkan/vulkan.hpp>
namespace backend
{
class BufferVK : public Buffer
{
public:
    explicit BufferVK(Device* device);
    ~BufferVK() override;
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
    DeviceVK* m_deviceVk{ nullptr };
    vk::Buffer m_buffer;
    vk::DeviceMemory m_deviceMemory;
    size_t m_bufferSize;
};
} // namespace backend

#endif // LEARNMETALVULKAN_BufferVK_H
