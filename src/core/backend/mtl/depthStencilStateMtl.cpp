//
// Created by william on 2022/12/6.
//

#include "depthStencilStateMtl.h"
namespace backend
{
DepthStencilStateMTL::DepthStencilStateMTL(Device* device) :
    DepthStencilState(device)
{
    m_deviceMtl = dynamic_cast<DeviceMtl*>(device);
    m_gpu = m_deviceMtl->gpu();
    m_descriptor = MTL::DepthStencilDescriptor::alloc()->init();
}

DepthStencilStateMTL::~DepthStencilStateMTL()
{
    m_depthStencilState->release();
}

MTL::DepthStencilState* DepthStencilStateMTL::handle()
{
    if (m_depthStencilState == nullptr)
    {
        m_depthStencilState = m_gpu->newDepthStencilState(m_descriptor);
        m_descriptor->release();
    }
    return m_depthStencilState;
}

static MTL::CompareFunction getCompareOpMtl(CompareOp op)
{
    MTL::CompareFunction compareFunc{};
    switch (op)
    {
    case CompareOp::Never:
        compareFunc = MTL::CompareFunction::CompareFunctionNever;
        break;
    case CompareOp::Less:
        compareFunc = MTL::CompareFunction::CompareFunctionLess;
        break;
    case CompareOp::Equal:
        compareFunc = MTL::CompareFunction::CompareFunctionEqual;
        break;
    case CompareOp::LessOrEqual:
        compareFunc = MTL::CompareFunction::CompareFunctionLessEqual;
        break;
    case CompareOp::Greater:
        compareFunc = MTL::CompareFunction::CompareFunctionGreater;
        break;
    case CompareOp::NotEqual:
        compareFunc = MTL::CompareFunction::CompareFunctionNotEqual;
        break;
    case CompareOp::GreaterOrEqual:
        compareFunc = MTL::CompareFunction::CompareFunctionGreaterEqual;
        break;
    case CompareOp::Always:
        compareFunc = MTL::CompareFunction::CompareFunctionAlways;
        break;
    }
    return compareFunc;
}

static MTL::StencilOperation getStencilOperationMtl(StencilOp stencilOp)
{
    MTL::StencilOperation stencilOperation;
    switch (stencilOp)
    {
    case StencilOp::Keep:
        stencilOperation = MTL::StencilOperation::StencilOperationKeep;
        break;
    case StencilOp::Zero:
        stencilOperation = MTL::StencilOperation::StencilOperationZero;
        break;
    case StencilOp::Replace:
        stencilOperation = MTL::StencilOperation::StencilOperationReplace;
        break;
    case StencilOp::IncrementAndClamp:
        stencilOperation = MTL::StencilOperation::StencilOperationIncrementClamp;
        break;
    case StencilOp::DecrementAndClamp:
        stencilOperation = MTL::StencilOperation::StencilOperationDecrementClamp;
        break;
    case StencilOp::Invert:
        stencilOperation = MTL::StencilOperation::StencilOperationInvert;
        break;
    case StencilOp::IncrementAndWrap:
        stencilOperation = MTL::StencilOperation::StencilOperationIncrementWrap;
        break;
    case StencilOp::DecrementAndWrap:
        stencilOperation = MTL::StencilOperation::StencilOperationDecrementWrap;
        break;
    }
    return stencilOperation;
}

MTL::StencilDescriptor* DepthStencilStateMTL::newStencilOpStateMtl(StencilOpState stencilOpState)
{
    MTL::StencilDescriptor* stencilDescriptor = MTL::StencilDescriptor::alloc()->init();
    stencilDescriptor->setReadMask(stencilOpState.compareMask);
    stencilDescriptor->setWriteMask(stencilOpState.writeMask);
    stencilDescriptor->setDepthFailureOperation(getStencilOperationMtl(stencilOpState.depthFailOp));
    stencilDescriptor->setStencilCompareFunction(getCompareOpMtl(stencilOpState.stencilCompareOp));
    stencilDescriptor->setStencilFailureOperation(getStencilOperationMtl(stencilOpState.stencilFailOp));
    stencilDescriptor->setDepthStencilPassOperation(getStencilOperationMtl(stencilOpState.depthStencilPassOp));
    return stencilDescriptor;
}

void DepthStencilStateMTL::setDepthCompareOp(CompareOp depthCompareOp)
{
    m_descriptor->setDepthCompareFunction(getCompareOpMtl(depthCompareOp));
}

void DepthStencilStateMTL::setStencilTestEnable(bool stencilTestEnable)
{
    DepthStencilState::setStencilTestEnable(stencilTestEnable);
}

void DepthStencilStateMTL::setFront(const StencilOpState& front)
{
    m_frontFaceStencil = newStencilOpStateMtl(front);
    m_descriptor->setFrontFaceStencil(m_frontFaceStencil);
    m_frontFaceStencil->release();
}

void DepthStencilStateMTL::setBack(const StencilOpState& back)
{
    m_backFaceStencil = newStencilOpStateMtl(back);
    m_descriptor->setBackFaceStencil(m_backFaceStencil);
    m_backFaceStencil->release();
}

void DepthStencilStateMTL::setDepthTestEnable(bool depthTestEnable)
{
    DepthStencilState::setDepthTestEnable(depthTestEnable);
}

void DepthStencilStateMTL::setDepthWriteEnable(bool depthWriteEnable)
{
    m_descriptor->setDepthWriteEnabled(depthWriteEnable);
}

void DepthStencilStateMTL::setDepthBoundsTestEnable(bool depthBoundsTestEnable)
{
    DepthStencilState::setDepthBoundsTestEnable(depthBoundsTestEnable);
}
} // namespace backend