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
#include "globalCommonDefine.h"
#include "mtlCommonDefine.h"
#include "pipeline.h"
#include "vkCommonDefine.h"

static std::vector<glm::vec4> getSphereMesh(int stacks = 16, int slices = 32, float radius = 1)
{
    using namespace glm;
    size_t vertexCount = ((stacks + 1) * (slices + 1));
    std::vector<vec3> vertices{ vertexCount };
    int index = 0;
    // create vertices
    for (int j = 0; j <= stacks; j++)
    {
        float latitude1 = (glm::pi<float>() / (float)stacks) * (float)j - (glm::pi<float>() / 2);
        float sinLat1 = sin(latitude1);
        float cosLat1 = cos(latitude1);
        for (int i = 0; i <= slices; i++)
        {
            float longitude = ((glm::pi<float>() * 2) / (float)slices) * (float)i;
            float sinLong = sin(longitude);
            float cosLong = cos(longitude);
            vec3 normal{ cosLong * cosLat1, sinLat1, sinLong * cosLat1 };
            normal = normalize(normal);
            vertices[index] = normal * radius;
            index++;
        }
    }
    std::vector<vec4> finalPosition;
    // create indices
    for (int j = 0; j < stacks; j++)
    {
        for (int i = 0; i <= slices; i++)
        {
            glm::u8vec2 offset[] = {
                // first triangle
                { i, j },
                { (i + 1) % (slices + 1), j + 1 },
                { (i + 1) % (slices + 1), j },

                // second triangle
                { i, j },
                { i, j + 1 },
                { (i + 1) % (slices + 1), j + 1 }
            };
            for (const auto& o : offset)
            {
                index = o[1] * (slices + 1) + o[0];
                finalPosition.push_back(glm::vec4(vertices[index], 1.0f));
            }
        }
    }
    return finalPosition;
}

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

struct alignas(16) FragLightingColorsUBO
{
    glm::vec4 lightColor{ 1.0f };
    glm::vec4 objectColor{ 1.0f };
};

struct alignas(16) FragBasicLightingColorUBO
{
    glm::vec4 lightPos{ 1.0f };
    glm::vec4 lightColor{ 1.0f };
    glm::vec4 objectColor{ 1.0f };
    glm::vec4 viewPos{ 1.0f };
};

struct alignas(16) Material
{
    glm::vec4 ambient{ 1.0f };
    glm::vec4 diffuse{ 1.0f };
    glm::vec4 specular{ 1.0f };
    float shininess{ 1.0f };
};

struct alignas(16) DiffuseMapMaterial
{
    float shininess{ 1.0f };
};

struct alignas(16) Light
{
    glm::vec4 position{ 1.0f };
    glm::vec4 ambient{ 1.0f };
    glm::vec4 diffuse{ 1.0f };
    glm::vec4 specular{ 1.0f };
};

struct alignas(16) FragLightSphereUBO
{
    glm::vec4 lightColor{ 1.0f };
};

struct alignas(16) FragMaterialsColorUBO
{
    glm::vec4 viewPos{ 1.0f };
    Material material;
    Light light;
};

struct alignas(16) FragDiffuseMapUBO
{
    glm::vec4 viewPos{ 1.0f };
    DiffuseMapMaterial material;
    Light light;
};

static constexpr uint32_t g_mvpMatrixUboBinding = 2;
static VertMVPMatrixUBO g_mvpMatrixUbo = { glm::eulerAngleZ(glm::radians(30.0f)), glm::mat4(1.0f), glm::mat4(1.0f) }; /* NOLINT */

static constexpr uint32_t g_lightingColorUboBinding = 3;
static FragLightingColorsUBO g_lightingColorsUbo = { glm::vec4(1.0f, 1.0f, 1.0f, 1.0), glm::vec4(1.0f, 0.5f, 0.31f, 1.0) }; /* NOLINT */

static constexpr uint32_t g_textureBinding = 1;

static constexpr uint32_t g_basicLightingColorUboBinding = 3;
static FragBasicLightingColorUBO g_basicLightingColorUbo = {
    { 1.2f, 1.0f, 2.0f, 1.0f },
    { 1.0f, 0.5f, 0.31f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f }
}; /* NOLINT */

static constexpr uint32_t g_lightSphereUboBinding = 4;
static FragLightSphereUBO g_lightColorUbo = {
    .lightColor = { 1.0f, 0.0f, 0.0f, 1.0f }
};

static constexpr uint32_t g_materialsUboBinding = 3;
static FragMaterialsColorUBO g_fragMaterialsColorUBO = {
    .viewPos = { 1.0f, 1.0f, 1.0f, 1.0f },
    .material = { { 1.0f, 0.5f, 0.31f, 1.0f },
                  { 1.0f, 0.5f, 0.31f, 1.0f },
                  { 0.5f, 0.5f, 0.5f, 1.0f },
                  32.0f },
    .light = { { 1.0f, 1.0f, 1.0f, 1.0f },
               { 1.0f, 1.0f, 1.0f, 1.0f },
               { 1.0f, 1.0f, 1.0f, 1.0f },
               { 1.0f, 1.0f, 1.0f, 1.0f } }
};

static constexpr uint32_t g_diffuseTextureBinding = 4;
static constexpr uint32_t g_specularTextureBinding = 5;

static constexpr uint32_t g_fragDiffuseMapUboBinding = 3;
static FragDiffuseMapUBO g_fragDiffuseMapUBO = {
    .viewPos = { 1.0f, 1.0f, 1.0f, 1.0f },
    .material = { 32.0f },
    .light = { { 1.0f, 1.0f, 1.0f, 1.0f },
               { 1.0f, 1.0f, 1.0f, 1.0f },
               { 1.0f, 1.0f, 1.0f, 1.0f },
               { 1.0f, 1.0f, 1.0f, 1.0f } }
};

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

static const std::vector<glm::vec4> g_sphereMesh = getSphereMesh();

struct alignas(16) LightingVertex
{
    glm::vec4 position;
    glm::vec4 normal;
};

struct alignas(16) LightingDiffuseMapVertex
{
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec4 texCoord;
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

static const std::vector<LightingDiffuseMapVertex> g_cubeVerticesWithNormalTexCoord = {
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },

    { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },

    { { -0.5f, 0.5f, 0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, 0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },

    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },

    { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } },
    { { -0.5f, 0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
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

static std::vector<backend::Pipeline::AttributeDescription> getOneElemAttributesDescriptions()
{
    static std::vector<backend::Pipeline::AttributeDescription> result{
        { .binding = 0,
          .stride = sizeof(glm::vec4),
          .inputRate = backend::InputRate::Vertex,
          .location = 0,
          .format = backend::Format::Float32,
          .offset = 0,
          .components = 4 }
    };
    return result;
}

static std::vector<backend::Pipeline::AttributeDescription> getTwoElemsAttributesDescriptions()
{
    static std::vector<backend::Pipeline::AttributeDescription> result = {
        { .binding = 0,
          .stride = sizeof(glm::vec4) * 2,
          .inputRate = backend::InputRate::Vertex,
          .location = 0,
          .format = backend::Format::Float32,
          .offset = 0,
          .components = 4 },
        { .binding = 0,
          .stride = sizeof(glm::vec4) * 2,
          .inputRate = backend::InputRate::Vertex,
          .location = 1,
          .format = backend::Format::Float32,
          .offset = 0 + sizeof(glm::vec4),
          .components = 4 }
    };
    return result;
}

static std::vector<backend::Pipeline::AttributeDescription> getThreeElemsAttributesDescriptions()
{
    static std::vector<backend::Pipeline::AttributeDescription> result = {
        { .binding = 0,
          .stride = sizeof(glm::vec4) * 3,
          .inputRate = backend::InputRate::Vertex,
          .location = 0,
          .format = backend::Format::Float32,
          .offset = 0,
          .components = 4 },
        { .binding = 0,
          .stride = sizeof(glm::vec4) * 3,
          .inputRate = backend::InputRate::Vertex,
          .location = 1,
          .format = backend::Format::Float32,
          .offset = 0 + sizeof(glm::vec4),
          .components = 4 },
        { .binding = 0,
          .stride = sizeof(glm::vec4) * 3,
          .inputRate = backend::InputRate::Vertex,
          .location = 2,
          .format = backend::Format::Float32,
          .offset = 0 + sizeof(glm::vec4) * 2,
          .components = 4 }
    };
    return result;
}

static std::vector g_textureShaderResource = {
    backend::ShaderResource{
        .binding = g_mvpMatrixUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Vertex,
        .arraySize = 1,
        .name = "VertMVPMatrixUBO" },
    backend::ShaderResource{
        .binding = g_textureBinding,
        .type = backend::ShaderResourceType::Sampler,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "inputTexture" }
};

static std::vector g_lightCubeShaderResource = {
    backend::ShaderResource{
        .binding = g_mvpMatrixUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Vertex,
        .arraySize = 1,
        .name = "VertMVPMatrixUBO" }
};

static std::vector g_basicLightingShaderResource = {
    backend::ShaderResource{
        .binding = g_mvpMatrixUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Vertex,
        .arraySize = 1,
        .name = "VertMVPMatrixUBO" },
    backend::ShaderResource{
        .binding = g_lightingColorUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "FragUniformBufferObject" }
};

static std::vector g_lightSphereShaderResource = {
    backend::ShaderResource{
        .binding = g_mvpMatrixUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Vertex,
        .arraySize = 1,
        .name = "VertMVPMatrixUBO" },
    backend::ShaderResource{
        .binding = g_lightSphereUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "FragUniformBufferObject" }
};

static std::vector g_materialsShaderResource = {
    backend::ShaderResource{
        .binding = g_mvpMatrixUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Vertex,
        .arraySize = 1,
        .name = "VertMVPMatrixUBO" },
    backend::ShaderResource{
        .binding = g_materialsUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "FragUniformBufferObject" }
};

static std::vector g_diffuseMapShaderResource = {
    backend::ShaderResource{
        .binding = g_mvpMatrixUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Vertex,
        .arraySize = 1,
        .name = "VertMVPMatrixUBO" },
    backend::ShaderResource{
        .binding = g_materialsUboBinding,
        .type = backend::ShaderResourceType::BufferUniform,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "FragUniformBufferObject" },
    backend::ShaderResource{
        .binding = g_diffuseTextureBinding,
        .type = backend::ShaderResourceType::Sampler,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "diffuseTexture" },
    backend::ShaderResource{
        .binding = g_specularTextureBinding,
        .type = backend::ShaderResourceType::Sampler,
        .mode = backend::ShaderResourceMode::Static,
        .stages = backend::ShaderType::Fragment,
        .arraySize = 1,
        .name = "specularTexture" }
};

#endif // LEARNMETALVULKAN_GLOBALMESHS_H
