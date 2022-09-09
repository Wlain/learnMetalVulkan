//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererMtl.h"

#include "deviceMtl.h"

namespace backend
{
GLFWRendererMtl::GLFWRendererMtl(Device* handle) :
    GLFWRenderer(handle)
{
    m_handleMtl = dynamic_cast<DeviceMtl*>(handle);
}

GLFWRendererMtl::~GLFWRendererMtl() = default;

void GLFWRendererMtl::swapBuffers()
{
}

void GLFWRendererMtl::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
}

} // namespace backend
