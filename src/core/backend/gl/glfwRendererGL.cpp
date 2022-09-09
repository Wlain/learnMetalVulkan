//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererGL.h"

#include <GLFW/glfw3.h>
namespace backend
{
GLFWRendererGL::~GLFWRendererGL() = default;

void GLFWRendererGL::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

void GLFWRendererGL::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
}

} // namespace backend