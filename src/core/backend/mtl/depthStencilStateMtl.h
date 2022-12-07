//
// Created by william on 2022/12/6.
//

#ifndef LEARNMETALVULKAN_DEPTHSTENCILSTATEMTL_H
#define LEARNMETALVULKAN_DEPTHSTENCILSTATEMTL_H
#include "depthStencilState.h"
#include "deviceMtl.h"
#include "mtlCommonDefine.h"
namespace backend
{
class DepthStencilStateMTL : public DepthStencilState
{
public:
    DepthStencilStateMTL(Device* device);
    ~DepthStencilStateMTL() override;
    [[nodiscard]] MTL::DepthStencilState* handle();
    void setDepthCompareOp(CompareOp depthCompareOp) override;
    void setStencilTestEnable(bool stencilTestEnable) override;
    void setFront(const StencilOpState& font) override;
    void setBack(const StencilOpState& back) override;
    void setDepthTestEnable(bool depthTestEnable) override;
    void setDepthWriteEnable(bool depthWriteEnable) override;
    void setDepthBoundsTestEnable(bool depthBoundsTestEnable) override;

private:
    MTL::StencilDescriptor* newStencilOpStateMtl(StencilOpState stencilOpState);

public:
    MTL::DepthStencilState* m_depthStencilState{ nullptr};
    DeviceMtl* m_deviceMtl{ nullptr };
    MTL::Device* m_gpu{ nullptr };
    MTL::DepthStencilDescriptor* m_descriptor{ nullptr };
    MTL::StencilDescriptor* m_frontFaceStencil{ nullptr };
    MTL::StencilDescriptor* m_backFaceStencil{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEPTHSTENCILSTATEMTL_H
