//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_VKCOMMONDEFINE_H
#define LEARNMETALVULKAN_VKCOMMONDEFINE_H
//#define VULKAN_HPP_NO_CONSTRUCTORS // 从 vulkan.hpp 中删除所有结构和联合构造函数
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
