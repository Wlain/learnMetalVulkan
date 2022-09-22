//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_VKCOMMONDEFINE_H
#define LEARNMETALVULKAN_VKCOMMONDEFINE_H

#include <vulkan/vulkan.hpp>

#define VK_CHECK(x)                                                 \
    do                                                              \
    {                                                               \
        VkResult err = x;                                           \
        if (err)                                                    \
        {                                                           \
            LOGE("Detected Vulkan error: {}", vkb::to_string(err)); \
            abort();                                                \
        }                                                           \
    } while (0)

#endif // LEARNMETALVULKAN_VKCOMMONDEFINE_H
