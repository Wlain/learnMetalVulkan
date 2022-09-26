//
// Created by cwb on 2022/9/8.
//

#include "deviceGL.h"

#include "glCommonDefine.h"
#include <GLFW/glfw3.h>
namespace backend
{
DeviceGL::~DeviceGL() = default;

DeviceGL::DeviceGL(const Info& info) :
    Device(info)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void DeviceGL::init()
{
    Device::init();
    glfwMakeContextCurrent(m_window);
    if (glewInit() != GLEW_OK)
    {
        LOG_ERROR("glewInit error");
        exit(EXIT_FAILURE);
    }
}
} // namespace backend