//
// Created by cwb on 2022/9/27.
//

#ifndef LEARNMETALVULKAN_BUFFERGL_H
#define LEARNMETALVULKAN_BUFFERGL_H

#include "buffer.h"
#include "deviceGL.h"
#include "glCommonDefine.h"

namespace backend
{
class BufferGL : public Buffer
{
public:
    explicit BufferGL(Device* device);
    ~BufferGL() override;
    void create(size_t size, void* data, BufferUsage usage, BufferType type) override;
    void update(void* data, size_t size, size_t offset) override;

private:
    GLuint m_buffer{ 0 };
};
} // namespace backend

#endif // LEARNMETALVULKAN_BUFFERGL_H
