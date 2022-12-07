//
// Created by william on 2022/12/6.
//

#ifndef LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
#define LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
#include "depthStencilState.h"
#include "glCommonDefine.h"
namespace backend
{
class DepthStencilStateGL : public DepthStencilState
{
public:
    explicit DepthStencilStateGL(Device* device);
    ~DepthStencilStateGL() override;

private:

};
} // namespace backend

#endif // LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
