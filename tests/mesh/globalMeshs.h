//
// Created by cwb on 2022/9/21.
//

#ifndef LEARNMETALVULKAN_GLOBALMESHS_H
#define LEARNMETALVULKAN_GLOBALMESHS_H
#include "glm/glm.hpp"

#include <vector>

struct alignas(16) TriangleVertex
{
    glm::vec4 position;
    glm::vec4 color;
};

// 按照std140规则,每个元素都对齐到 16 字节
const std::vector<TriangleVertex> g_triangleVertex{
    { { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0, 0.0f, 0.0f, 1.0f } },
    { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0, 0.0f, 1.0f } },
    { { 0.0f, 0.5f, 0.0f, 1.0 }, { 0.0f, 0.0f, 1.0, 1.0f } }
};

#endif // LEARNMETALVULKAN_GLOBALMESHS_H
