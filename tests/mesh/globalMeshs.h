//
// Created by cwb on 2022/9/21.
//

#ifndef LEARNMETALVULKAN_GLOBALMESHS_H
#define LEARNMETALVULKAN_GLOBALMESHS_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <vector>
#define VULKAN_HPP_NO_CONSTRUCTORS // 从 vulkan.hpp 中删除所有结构和联合构造函数
#include "mtlCommonDefine.h"
#include "vkCommonDefine.h"

struct alignas(16) TriangleVertex
{
    glm::vec4 position;
    glm::vec4 color;
};

// 按照std140规则,每个元素都对齐到 16 字节
static const std::vector<TriangleVertex> g_triangleVertex{ /* NOLINT */
                                                           { { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0, 0.0f, 0.0f, 1.0f } },
                                                           { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0, 0.0f, 1.0f } },
                                                           { { 0.0f, 0.5f, 0.0f, 1.0 }, { 0.0f, 0.0f, 1.0, 1.0f } }
};

struct alignas(16) TextureVertex
{
    glm::vec4 position;
    glm::vec4 texCoord;
};

struct alignas(16) VertMVPMatrixUBO
{
    glm::mat4 model{ 1.0f };
    glm::mat4 view{ 1.0f };
    glm::mat4 proj{ 1.0f };
};

struct alignas(16) FragLightingColorUBO
{
    glm::vec4 lightColor{ 1.0f };
    glm::vec4 objectColor{ 1.0f };
};

struct alignas(16) FragBasicLightingColorUBO
{
    glm::vec4 lightPos{ 1.0f };
    glm::vec4 lightColor{ 1.0f };
    glm::vec4 objectColor{ 1.0f };
};

static constexpr uint32_t g_mvpMatrixUboBinding = 2;
static VertMVPMatrixUBO g_mvpMatrixUbo = { glm::eulerAngleZ(glm::radians(30.0f)), glm::mat4(1.0f), glm::mat4(1.0f) }; /* NOLINT */

static constexpr uint32_t g_lightingColorUboBinding = 3;

static FragLightingColorUBO g_lightingColorUbo = { glm::vec4(1.0f, 1.0f, 1.0f, 1.0), glm::vec4(1.0f, 0.5f, 0.31f, 1.0) }; /* NOLINT */

static constexpr uint32_t g_textureBinding = 1;

static constexpr uint32_t g_basicLightingColorUboBinding = 3;
static FragBasicLightingColorUBO g_basicLightingColorUbo = { glm::vec4(1.2f, 1.0f, 2.0f, 1.0), glm::vec4(1.0f, 0.5f, 0.31f, 1.0), glm::vec4(1.0f, 1.0f, 1.0f, 1.0) }; /* NOLINT */

static glm::vec3 g_lightPos{ 1.2f, 1.0f, 2.0f };

static const std::vector<TextureVertex> g_quadVertex = {
    /* NOLINT */
    // positions                  // texture coords
    { { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },  // top left
    { { 0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },   // top right
    { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } }, // bottom left
    { { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } }   // bottom right
};

static const std::vector<uint16_t> g_quadIndices = { /* NOLINT */
                                                     1, 3, 0,
                                                     3, 2, 0
};

static const std::vector<TextureVertex> g_cubeVertex = { /* NOLINT */
                                                         // positions                  // texture coords
                                                         { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },

                                                         { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },

                                                         { { -0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },

                                                         { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },

                                                         { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },

                                                         { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
                                                         { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } }
};

static const std::vector<glm::vec4> g_cubeVertices = {
    { -0.5f, -0.5f, -0.5f, 1.0f },
    { 0.5f, -0.5f, -0.5f, 1.0f },
    { 0.5f, 0.5f, -0.5f, 1.0f },
    { 0.5f, 0.5f, -0.5f, 1.0f },
    { -0.5f, 0.5f, -0.5f, 1.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f },

    { -0.5f, -0.5f, 0.5f, 1.0f },
    { 0.5f, -0.5f, 0.5f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { -0.5f, 0.5f, 0.5f, 1.0f },
    { -0.5f, -0.5f, 0.5f, 1.0f },

    { -0.5f, 0.5f, 0.5f, 1.0f },
    { -0.5f, 0.5f, -0.5f, 1.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f },
    { -0.5f, -0.5f, 0.5f, 1.0f },
    { -0.5f, 0.5f, 0.5f, 1.0f },

    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 0.5f, 0.5f, -0.5f, 1.0f },
    { 0.5f, -0.5f, -0.5f, 1.0f },
    { 0.5f, -0.5f, -0.5f, 1.0f },
    { 0.5f, -0.5f, 0.5f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },

    { -0.5f, -0.5f, -0.5f, 1.0f },
    { 0.5f, -0.5f, -0.5f, 1.0f },
    { 0.5f, -0.5f, 0.5f, 1.0f },
    { 0.5f, -0.5f, 0.5f, 1.0f },
    { -0.5f, -0.5f, 0.5f, 1.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f },

    { -0.5f, 0.5f, -0.5f, 1.0f },
    { 0.5f, 0.5f, -0.5f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { -0.5f, 0.5f, 0.5f, 1.0f },
    { -0.5f, 0.5f, -0.5f, 1.0f }
};

struct alignas(16) LightingVertex
{
    glm::vec4 position;
    glm::vec4 normal;
};

static const std::vector<LightingVertex> g_cubeVerticesWithNormal = {
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f } },

    { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },
    { { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },

    { { -0.5f, 0.5f, 0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, 0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f } },

    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f } },

    { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } }
};

// world space positions of our cubes
static const std::vector<glm::vec3> g_cubePositions = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(2.0f, 5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f, 3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f, 2.0f, -2.5f),
    glm::vec3(1.5f, 0.2f, -1.5f),
    glm::vec3(-1.3f, 1.0f, -1.5f)
};

static vk::VertexInputBindingDescription getPosTexCoordBindingDescription()
{
    auto bindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(TriangleVertex),
        .inputRate = vk::VertexInputRate::eVertex
    };
    return bindingDescription;
}

static std::array<vk::VertexInputAttributeDescription, 2> getPosTexCoordAttributeDescriptions()
{
    auto attributeDescriptions = std::array<vk::VertexInputAttributeDescription, 2>{
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(TriangleVertex, position) },
        vk::VertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(TriangleVertex, color) },
    };
    return attributeDescriptions;
}

static vk::VertexInputBindingDescription getPosBindingDescription()
{
    auto bindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(glm::vec4),
        .inputRate = vk::VertexInputRate::eVertex
    };
    return bindingDescription;
}

static std::array<vk::VertexInputAttributeDescription, 1> getPosAttributeDescriptions()
{
    auto attributeDescriptions = std::array<vk::VertexInputAttributeDescription, 1>{
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = 0 }
    };
    return attributeDescriptions;
}

static MTL::VertexDescriptor* getPosTexCoordVertexDescriptor()
{
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    // texCoord
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(1)->setOffset(16);
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    // layout
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(glm::vec4) * 2);
    return vertexDescriptor;
}

static MTL::VertexDescriptor* getPosVertexDescriptor()
{
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    // layout
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(glm::vec4));
    return vertexDescriptor;
}

#endif // LEARNMETALVULKAN_GLOBALMESHS_H
