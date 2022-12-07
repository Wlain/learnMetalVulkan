//
// Created by william on 2022/12/6.
//

#ifndef LEARNMETALVULKAN_DEPTHSTENCILSTATEVK_H
#define LEARNMETALVULKAN_DEPTHSTENCILSTATEVK_H
#include "depthStencilState.h"
#include "vkCommonDefine.h"
namespace backend
{
class DepthStencilStateVk : public DepthStencilState
{
public:
    DepthStencilStateVk(Device* device);
    ~DepthStencilStateVk() override;
    void setDepthCompareOp(CompareOp mDepthCompareOp) override;
    void setStencilTestEnable(bool mStencilTestEnable) override;
    void setFront(const StencilOpState& mFront) override;
    void setBack(const StencilOpState& mBack) override;
    void setDepthTestEnable(bool mDepthTestEnable) override;
    void setDepthWriteEnable(bool mDepthWriteEnable) override;
    void setDepthBoundsTestEnable(bool mDepthBoundsTestEnable) override;

public:
    [[nodiscard]] const vk::PipelineDepthStencilStateCreateInfo& handle() const;

private:
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilState;
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEPTHSTENCILSTATEVK_H
