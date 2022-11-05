//
// Created by william on 2022/11/4.
//

#ifndef LEARNMETALVULKAN_GLOBALCOMMONDEFINE_H
#define LEARNMETALVULKAN_GLOBALCOMMONDEFINE_H
#include <cstdint>
#include <string>

namespace backend
{
enum class ShaderResourceType : uint8_t
{
    Unknown,
    Input,
    InputAttachment,
    Output,
    Image,
    ImageSampler,
    ImageStorage,
    Sampler,
    BufferUniform,
    BufferStorage,
    PushConstant,
    SpecializationConstant,
    All
};

enum class ShaderResourceMode : uint8_t
{
    Unknown,
    Static,
    Dynamic,
    UpdateAfterBind
};

enum class ShaderType : uint8_t
{
    Unknown,
    Vertex,
    Fragment
};

struct ShaderResource
{
    ShaderType stages{};
    ShaderResourceType type{};
    ShaderResourceMode mode{};
    uint32_t set{ 0 };
    uint32_t binding{ 0 };
    uint32_t location{ 0 };
    uint32_t inputAttachmentIndex{ 0 };
    uint32_t vecSize{ 0 };
    uint32_t columns{ 0 };
    uint32_t arraySize{ 0 };
    uint32_t offset{ 0 };
    uint32_t size{ 0 };
    uint32_t constantId{ 0 };
    uint32_t qualifiers{ 0 };
    std::string name;
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLOBALCOMMONDEFINE_H
