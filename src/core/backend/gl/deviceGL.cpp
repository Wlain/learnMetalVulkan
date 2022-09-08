//
// Created by cwb on 2022/9/8.
//

#include "deviceGL.h"

#include <GLFW/glfw3.h>
namespace backend
{
HandleGL::~HandleGL() = default;

HandleGL::HandleGL(const Info& info) :
    Device(info)
{
    m_info.type = RenderType::OpenGL;
}

void HandleGL::init()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}
} // namespace backend