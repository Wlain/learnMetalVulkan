//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERVK_H
#define LEARNMETALVULKAN_GLFWRENDERERVK_H
#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include "glfwRenderer.h"

class GLFWRendererVK : public GLFWRenderer
{
public:
    using GLFWRenderer::GLFWRenderer;
    ~GLFWRendererVK() override;

public:
    void initGlfw() override;
    void swapWindow() override;
    void initSwapChain() override;

private:
    VkInstance m_instance{ nullptr };
    VkSurfaceKHR m_surface{ nullptr };
    VkDevice m_device{ nullptr};
};

#endif // LEARNMETALVULKAN_GLFWRENDERERVK_H
