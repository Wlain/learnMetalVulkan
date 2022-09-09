//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERMTL_H
#define LEARNMETALVULKAN_GLFWRENDERERMTL_H

#include "glfwRenderer.h"
namespace backend
{
class DeviceMtl;
class GLFWRendererMtl : public GLFWRenderer
{
public:
    explicit GLFWRendererMtl(Device* handle);
    ~GLFWRendererMtl() override;
    void swapBuffers() override;
    void setPipeline(const std::shared_ptr<Pipeline>& pipeline) override;

private:
    DeviceMtl* m_handleMtl;
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERERMTL_H
