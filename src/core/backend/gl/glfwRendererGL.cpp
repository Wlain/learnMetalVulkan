//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererGL.h"
namespace backend
{
GLFWRendererGL::~GLFWRendererGL() = default;

void GLFWRendererGL::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

void GLFWRendererGL::initSwapChain()
{
    glfwMakeContextCurrent(m_window);
}

void GLFWRendererGL::setPipeline(const Pipeline& pipeline)
{
}

} // namespace backend