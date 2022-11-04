//
// Created by william on 2022/11/4.
//

#ifndef LEARNMETALVULKAN_GLOBALCOMMONDEFINE_H
#define LEARNMETALVULKAN_GLOBALCOMMONDEFINE_H
#include <cstdint>

enum class ShaderResourceType : uint8_t
{
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
    Static,
    Dynamic,
    UpdateAfterBind
};

enum class ShaderType : uint8_t
{
    Vertex,
    Fragment
};

struct ShaderResource
{
    ShaderType stages;
    ShaderResourceType type;
    ShaderResourceMode mode;
    uint32_t set;
    uint32_t binding;
    uint32_t location;
    uint32_t inputAttachmentIndex;
    uint32_t vecSize;
    uint32_t columns;
    uint32_t arraySize;
    uint32_t offset;
    uint32_t size;
    uint32_t constantId;
    uint32_t qualifiers;
    std::string name;
};

#endif // LEARNMETALVULKAN_GLOBALCOMMONDEFINE_H
