//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_DEVICE_H
#define LEARNMETALVULKAN_DEVICE_H

#include <cstdint>
#include <string>

struct GLFWwindow;
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
        std::string title;
    };

public:
    explicit Device(const Info& info);
    virtual ~Device();
    RenderType type() const;
    uint32_t width() const;
    uint32_t height() const;
    GLFWwindow* window() const;
    virtual void init();

protected:
    Info m_info;
    GLFWwindow* m_window{ nullptr };
};
} // namespace backend
#endif // LEARNMETALVULKAN_HANDLE_H
