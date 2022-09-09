//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERGL_H
#define LEARNMETALVULKAN_GLFWRENDERERGL_H

#include "glfwRenderer.h"
namespace backend
{
class GLFWRendererGL : public GLFWRenderer
{
public:
    using GLFWRenderer::GLFWRenderer;
    ~GLFWRendererGL() override;
    void setPipeline(const std::shared_ptr<Pipeline>& pipeline) override;
    void swapBuffers() override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERERGL_H
