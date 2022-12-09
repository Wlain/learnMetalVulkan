//
// Created by william on 2022/12/6.
//

#ifndef LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
#define LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
#include "depthStencilState.h"
#include "glCommonDefine.h"
namespace backend
{
class DepthStencilStateGl : public DepthStencilState
{
public:
    explicit DepthStencilStateGl(Device* device);
    ~DepthStencilStateGl() override;
    static GLenum getCompareOpGl(CompareOp op);
    static GLenum getStencilOperationGl(StencilOp stencilOp);
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEPTHSTENCILSTATEGL_H
