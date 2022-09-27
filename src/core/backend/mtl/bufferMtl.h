//
// Created by william on 2022/9/27.
//

#ifndef LEARNMETALVULKAN_BUFFERMtl_H
#define LEARNMETALVULKAN_BUFFERMtl_H
#include "buffer.h"
#include "deviceMtl.h"

namespace MTL
{
class Buffer;
}

namespace backend
{
class BufferMTL : public Buffer
{
public:
    explicit BufferMTL(Device* device);
    ~BufferMTL() override;
    void create(size_t size, void* data, BufferUsage usage, BufferType type) override;
    void update(void* data, size_t size, size_t offset) override;
    MTL::Buffer* buffer() const;

private:
    DeviceMtl* m_deviceMtl{ nullptr };
    MTL::Buffer* m_buffer{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_BufferMtl_H
