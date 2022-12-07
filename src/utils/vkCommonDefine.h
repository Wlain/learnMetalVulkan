//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_VKCOMMONDEFINE_H
#define LEARNMETALVULKAN_VKCOMMONDEFINE_H
#define VULKAN_HPP_NO_CONSTRUCTORS // 从 vulkan.hpp 中删除所有结构和联合构造函数
#include <vulkan/vulkan.hpp>
#ifndef VULKAN_HPP_TYPESAFE_CONVERSION
    #define VULKAN_HPP_TYPESAFE_CONVERSION 1
#endif

#define VK_CHECK(func)                   \
    do                                   \
    {                                    \
        vk::Result res = func;           \
        if (res != vk::Result::eSuccess) \
        {                                \
            ASSERT(0);                   \
        }                                \
    } while (0)
#endif // LEARNMETALVULKAN_VKCOMMONDEFINE_H
