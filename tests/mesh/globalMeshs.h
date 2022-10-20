//
// Created by cwb on 2022/9/21.
//

#ifndef LEARNMETALVULKAN_GLOBALMESHS_H
#define LEARNMETALVULKAN_GLOBALMESHS_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <vector>

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

struct alignas(16) UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

static UniformBufferObject g_mvpMatrix = { glm::eulerAngleZ(glm::radians(30.0f)), glm::mat4(1.0f), glm::mat4(1.0f) }; /* NOLINT */

// clang-format off
static const std::vector<TextureVertex> g_quadVertex = {    /* NOLINT */
    // positions                  // texture coords
    { { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },  // top left
    { { 0.5f, 0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 0.0f } },   // top right
    { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } }, // bottom left
    { { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } }   // bottom right
};
// clang-format on

static const std::vector<uint16_t> g_quadIndices = { /* NOLINT */
                                                     1, 3, 0,
                                                     3, 2, 0
};

#endif // LEARNMETALVULKAN_GLOBALMESHS_H
