//
// Created by cwb on 2022/9/27.
//

#include "bufferGL.h"

namespace backend
{

BufferGL::BufferGL(backend::Device* device) :
    Buffer(device)
{
    glGenBuffers(1, &m_buffer);
}

BufferGL::~BufferGL()
{
    glDeleteBuffers(1, &m_buffer);
}

GLenum getBufferUsage(Buffer::BufferUsage usage)
{
    GLenum bufferUsage{};
    switch (usage)
    {
    case Buffer::BufferUsage::StaticDraw:
        bufferUsage = GL_STATIC_DRAW;
        break;
    case Buffer::BufferUsage::DynamicDraw:
        bufferUsage = GL_DYNAMIC_DRAW;
        break;
    case Buffer::BufferUsage::StreamDraw:
        bufferUsage = GL_STREAM_DRAW;
        break;
    }
    return bufferUsage;
}

GLenum getBufferType(Buffer::BufferType type)
{
    GLenum bufferType{};
    switch (type)
    {
    case Buffer::BufferType::VertexBuffer:
        bufferType = GL_ARRAY_BUFFER;
        break;
    case Buffer::BufferType::IndexBuffer:
        bufferType = GL_ELEMENT_ARRAY_BUFFER;
        break;
    case Buffer::BufferType::UniformBuffer:
        bufferType = GL_UNIFORM_BUFFER;
        break;
    }
    return bufferType;
}

void BufferGL::create(size_t size, void* data, Buffer::BufferUsage usage, Buffer::BufferType type)
{
    auto bufferUsage = getBufferUsage(usage);
    auto bufferType = getBufferType(type);
    glBindBuffer(bufferType, m_buffer);
    glBufferData(bufferType, size, data, bufferUsage);
}

void BufferGL::update(void* data, size_t size, size_t offset)
{
    Buffer::update(data, size, offset);
}
} // namespace backend