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
    static GLenum getCompareOpGl(CompareOp op);
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
