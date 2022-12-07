//
// Created by william on 2022/12/6.
//

#include "depthStencilState.h"

namespace backend
{

DepthStencilState::DepthStencilState(Device* device)
{
}

DepthStencilState::~DepthStencilState() = default;

CompareOp DepthStencilState::depthCompareOp() const
{
    return m_depthCompareOp;
}

const StencilOpState& DepthStencilState::front() const
{
    return m_front;
}

const StencilOpState& DepthStencilState::back() const
{
    return m_back;
}

bool DepthStencilState::depthTestEnable() const
{
    return m_depthTestEnable;
}

bool DepthStencilState::depthWriteEnable() const
{
    return m_depthWriteEnable;
}

bool DepthStencilState::depthBoundsTestEnable() const
{
    return m_depthBoundsTestEnable;
}

bool DepthStencilState::stencilTestEnable() const
{
    return m_stencilTestEnable;
}

void DepthStencilState::setDepthCompareOp(CompareOp depthCompareOp)
{
    m_depthCompareOp = depthCompareOp;
}

void DepthStencilState::setFront(const StencilOpState& front)
{
    m_front = front;
}

void DepthStencilState::setBack(const StencilOpState& back)
{
    m_back = back;
}

void DepthStencilState::setDepthTestEnable(bool depthTestEnable)
{
    m_depthTestEnable = depthTestEnable;
}

void DepthStencilState::setDepthWriteEnable(bool depthWriteEnable)
{
    m_depthWriteEnable = depthWriteEnable;
}

void DepthStencilState::setDepthBoundsTestEnable(bool depthBoundsTestEnable)
{
    m_depthBoundsTestEnable = depthBoundsTestEnable;
}

void DepthStencilState::setStencilTestEnable(bool stencilTestEnable)
{
    m_stencilTestEnable = stencilTestEnable;
}
} // namespace backend