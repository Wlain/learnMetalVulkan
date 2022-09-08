//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererMtl.h"

#include "deviceMtl.h"

namespace backend
{
extern void* createLayer(GLFWwindow* window, double width, double height, void* device);

GLFWRendererMtl::GLFWRendererMtl(Device* handle) :
    GLFWRenderer(handle)
{
    m_handleMtl = dynamic_cast<DeviceMtl*>(handle);
}

GLFWRendererMtl::~GLFWRendererMtl() = default;

void GLFWRendererMtl::swapBuffers()
{
}

void GLFWRendererMtl::initSwapChain()
{
    m_gpu = MTL::CreateSystemDefaultDevice();
    m_swapChain = (CA::MetalLayer*)createLayer(m_window, m_windowWidth, m_windowHeight, m_gpu);
    m_queue = m_gpu->newCommandQueue();
}

CA::MetalLayer* GLFWRendererMtl::swapChain() const
{
    return m_swapChain;
}

MTL::Device* GLFWRendererMtl::gpu() const
{
    return m_gpu;
}

MTL::CommandQueue* GLFWRendererMtl::queue() const
{
    return m_queue;
}

void GLFWRendererMtl::setPipeline(const Pipeline& pipeline)
{
}

} // namespace backend
