//
// Created by cwb on 2022/9/8.
//
#include "deviceMtl.h"

#include "commonMacro.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
namespace backend
{
extern void* createLayer(GLFWwindow* window, double width, double height, void* device);

DeviceMtl::~DeviceMtl() = default;

DeviceMtl::DeviceMtl(const Info& info) :
    Device(info)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void DeviceMtl::init()
{
    Device::init();
    m_gpu = MTL::CreateSystemDefaultDevice();
    if (!m_gpu)
    {
        LOG_ERROR("DeviceMtl::create Device failed!!");
    }
    m_swapChain = (CA::MetalLayer*)createLayer(m_window, m_info.width, m_info.height, m_gpu);
    m_queue = m_gpu->newCommandQueue();
}

CA::MetalLayer* DeviceMtl::swapChain() const
{
    return m_swapChain;
}

MTL::Device* DeviceMtl::gpu() const
{
    return m_gpu;
}

MTL::CommandQueue* DeviceMtl::queue() const
{
    return m_queue;
}
} // namespace backend
