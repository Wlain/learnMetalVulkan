//
// Created by cwb on 2022/9/8.
//

#include "deviceMtl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace backend
{
DeviceMtl::~DeviceMtl() = default;

DeviceMtl::DeviceMtl(const Info& info) :
    Device(info)
{
    m_info.type = RenderType::Metal;
}

void DeviceMtl::init()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}
} // namespace backend
