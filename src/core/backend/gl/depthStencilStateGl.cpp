//
// Created by william on 2022/12/6.
//

#include "depthStencilStateGl.h"
namespace backend
{
DepthStencilStateGL::DepthStencilStateGL(Device* device) :
    DepthStencilState(device)
{
}

DepthStencilStateGL::~DepthStencilStateGL() = default;
} // namespace backend