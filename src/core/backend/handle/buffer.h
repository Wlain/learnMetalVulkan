//
// Created by cwb on 2022/9/26.
//

#ifndef LEARNMETALVULKAN_BUFFER_H
#define LEARNMETALVULKAN_BUFFER_H
#include "device.h"
namespace backend
{
class Buffer
{
public:
    enum class BufferUsage
    {
        StaticDraw,
        DynamicDraw,
        StreamDraw
    };

    enum class DataFormat
    {
        Int,
        Float,
        Vec2,
        Vec3,
        Vec4,
        Mat2,
        Mat3,
        Mat4
    };

    enum class BufferType
    {
        VertexBuffer,
        IndexBuffer,  //
        UniformBuffer // Ubo
    };

public:
    explicit Buffer(Device* device);
    virtual ~Buffer();

    virtual void create(size_t size, void* data, BufferUsage usage, BufferType type);
    virtual void update(void* data, size_t size, size_t offset);

protected:
    Device* m_device{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_BUFFER_H
