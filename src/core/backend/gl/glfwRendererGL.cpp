//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererGL.h"

GLFWRendererGL::~GLFWRendererGL() = default;

void GLFWRendererGL::initGlfw()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void GLFWRendererGL::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

void GLFWRendererGL::initSwapChain()
{
    glfwMakeContextCurrent(m_window);
}
