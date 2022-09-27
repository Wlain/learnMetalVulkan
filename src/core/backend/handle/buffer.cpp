//
// Created by cwb on 2022/9/26.
//

#include "buffer.h"

namespace backend
{
Buffer::Buffer(backend::Device* device)
{
}
Buffer::~Buffer() = default;

void Buffer::create(size_t size, void* data, BufferUsage usage, BufferType type)
{
}

void Buffer::update(void* data, size_t size, size_t offset)
{
}
} // namespace backend