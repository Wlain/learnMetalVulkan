//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVK.h"
GLFWRendererVK::~GLFWRendererVK()
{
    // 销毁surface对象
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    // 销毁window
    glfwDestroyWindow(m_window);
    // 销毁device 和 instance
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void GLFWRendererVK::initGlfw()
{
    // This tells GLFW to not create an OpenGL context with the window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void GLFWRendererVK::swapWindow()
{
}

void GLFWRendererVK::initSwapChain()
{
}
