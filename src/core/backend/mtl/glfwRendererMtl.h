//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERERMTL_H
#define LEARNMETALVULKAN_GLFWRENDERERMTL_H

#include "glfwRenderer.h"
#define GLFW_INCLUDE_NONE
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
namespace backend
{
class DeviceMtl;
class GLFWRendererMtl : public GLFWRenderer
{
public:
    explicit GLFWRendererMtl(Device* handle);
    ~GLFWRendererMtl() override;
    void swapBuffers() override;
    void setPipeline(const Pipeline& pipeline) override;
    void initSwapChain() override;
    CA::MetalLayer* swapChain() const;
    MTL::Device* gpu() const;
    MTL::CommandQueue* queue() const;

private:
    CA::MetalLayer* m_swapChain{ nullptr };
    MTL::Device* m_gpu{ nullptr };
    MTL::CommandQueue* m_queue{ nullptr };
    DeviceMtl* m_handleMtl;
};
}

#endif // LEARNMETALVULKAN_GLFWRENDERERMTL_H
