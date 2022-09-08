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

uint32_t Device::getWidth() const
{
    return m_info.width;
}

uint32_t Device::getHeight() const
{
    return m_info.height;
}
} // namespace backend
