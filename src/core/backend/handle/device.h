//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_DEVICE_H
#define LEARNMETALVULKAN_DEVICE_H

#include <cstdint>
class GLFWwindow;
namespace backend
{
class Device
{
public:
    enum class RenderType : uint8_t
    {
        None,
        OpenGL,
        Vulkan,
        Metal
    };

    struct Info
    {
        RenderType type{ RenderType::None };
        uint32_t width{ 0 };
        uint32_t height{ 0 };
    };

public:
    Device(const Info& info);
    virtual ~Device() = default;
    RenderType type() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    GLFWwindow* window() const;
    virtual void init() = 0;

protected:
    Info m_info;
    GLFWwindow* m_window{ nullptr };
};
}
#endif // LEARNMETALVULKAN_HANDLE_H
