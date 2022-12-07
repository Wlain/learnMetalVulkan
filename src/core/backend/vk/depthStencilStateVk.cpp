//
// Created by william on 2022/12/6.
//

#include "depthStencilStateVk.h"

namespace backend
{
DepthStencilStateVk::DepthStencilStateVk(Device* device) :
    DepthStencilState(device)
{
}

DepthStencilStateVk::~DepthStencilStateVk() = default;

 const vk::PipelineDepthStencilStateCreateInfo& DepthStencilStateVk::handle() const
{
     return m_depthStencilState;
 }

static vk::CompareOp getCompareOpVk(CompareOp op)
{
    vk::CompareOp compareFunc{};
    switch (op)
    {
    case CompareOp::Never:
        compareFunc = vk::CompareOp::eNever;
        break;
    case CompareOp::Less:
        compareFunc = vk::CompareOp::eLess;
        break;
    case CompareOp::Equal:
        compareFunc = vk::CompareOp::eEqual;
        break;
    case CompareOp::LessOrEqual:
        compareFunc = vk::CompareOp::eLessOrEqual;
        break;
    case CompareOp::Greater:
        compareFunc = vk::CompareOp::eGreater;
        break;
    case CompareOp::NotEqual:
        compareFunc = vk::CompareOp::eNotEqual;
        break;
    case CompareOp::GreaterOrEqual:
        compareFunc = vk::CompareOp::eGreaterOrEqual;
        break;
    case CompareOp::Always:
        compareFunc = vk::CompareOp::eAlways;
        break;
    }
    return compareFunc;
}

static vk::StencilOp getStencilOperationVk(StencilOp stencilOp)
{
    vk::StencilOp stencilOperation;
    switch (stencilOp)
    {
    case StencilOp::Keep:
        stencilOperation = vk::StencilOp::eKeep;
        break;
    case StencilOp::Zero:
        stencilOperation = vk::StencilOp::eZero;
        break;
    case StencilOp::Replace:
        stencilOperation = vk::StencilOp::eReplace;
        break;
    case StencilOp::IncrementAndClamp:
        stencilOperation = vk::StencilOp::eIncrementAndClamp;
        break;
    case StencilOp::DecrementAndClamp:
        stencilOperation = vk::StencilOp::eDecrementAndClamp;
        break;
    case StencilOp::Invert:
        stencilOperation = vk::StencilOp::eInvert;
        break;
    case StencilOp::IncrementAndWrap:
        stencilOperation = vk::StencilOp::eIncrementAndWrap;
        break;
    case StencilOp::DecrementAndWrap:
        stencilOperation = vk::StencilOp::eDecrementAndWrap;
        break;
    }
    return stencilOperation;
}

void DepthStencilStateVk::setDepthCompareOp(CompareOp depthCompareOp)
{
    m_depthStencilState.setDepthCompareOp(getCompareOpVk(depthCompareOp));
}

void DepthStencilStateVk::setStencilTestEnable(bool stencilTestEnable)
{
    m_depthStencilState.setStencilTestEnable(stencilTestEnable);
}

void DepthStencilStateVk::setFront(const StencilOpState& front)
{
    vk::StencilOpState backStencilOpState{
        .failOp = getStencilOperationVk(front.stencilFailOp),
        .passOp = getStencilOperationVk(front.depthStencilPassOp),
        .depthFailOp = getStencilOperationVk(front.depthFailOp),
        .compareOp = getCompareOpVk(front.stencilCompareOp),
        .writeMask = front.writeMask,
        .compareMask = front.compareMask,
        .reference = front.reference
    };
    m_depthStencilState.setBack(backStencilOpState);
}

void DepthStencilStateVk::setBack(const StencilOpState& back)
{
    vk::StencilOpState backStencilOpState{
        .failOp = getStencilOperationVk(back.stencilFailOp),
        .passOp = getStencilOperationVk(back.depthStencilPassOp),
        .depthFailOp = getStencilOperationVk(back.depthFailOp),
        .compareOp = getCompareOpVk(back.stencilCompareOp),
        .writeMask = back.writeMask,
        .compareMask = back.compareMask,
        .reference = back.reference
    };
    m_depthStencilState.setBack(backStencilOpState);
}

void DepthStencilStateVk::setDepthTestEnable(bool depthTestEnable)
{
    m_depthStencilState.setDepthTestEnable(depthTestEnable);
}

void DepthStencilStateVk::setDepthWriteEnable(bool depthWriteEnable)
{
    m_depthStencilState.setDepthWriteEnable(depthWriteEnable);
}

void DepthStencilStateVk::setDepthBoundsTestEnable(bool depthBoundsTestEnable)
{
    m_depthStencilState.setDepthBoundsTestEnable(depthBoundsTestEnable);
}
} // namespace backend