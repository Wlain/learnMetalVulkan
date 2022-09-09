//
// Created by cwb on 2022/9/8.
//

#include "device.h"

#include <GLFW/glfw3.h>
namespace backend
{
Device::Device(const Info& info) :
    m_info(info)
{
    glfwInit();
}

Device::RenderType Device::type() const
{
    return m_info.type;
}

GLFWwindow* Device::window() const
{
    return m_window;
}

uint32_t Device::width() const
{
    return m_info.width;
}

uint32_t Device::height() const
{
    return m_info.height;
}

void Device::init()
{
    m_window = glfwCreateWindow(m_info.width, m_info.height, m_info.title.c_str(), nullptr, nullptr);
}
} // namespace backend
