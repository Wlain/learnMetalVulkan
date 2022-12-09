//
// Created by william on 2022/12/6.
//

#include "depthStencilStateGl.h"
namespace backend
{
DepthStencilStateGl::DepthStencilStateGl(Device* device) :
    DepthStencilState(device)
{
}

GLenum DepthStencilStateGl::getCompareOpGl(CompareOp op)
{
    GLenum compareFunc{};
    switch (op)
    {
    case CompareOp::Never:
        compareFunc = GL_NEVER;
        break;
    case CompareOp::Less:
        compareFunc = GL_LESS;
        break;
    case CompareOp::Equal:
        compareFunc = GL_EQUAL;
        break;
    case CompareOp::LessOrEqual:
        compareFunc = GL_LEQUAL;
        break;
    case CompareOp::Greater:
        compareFunc = GL_GREATER;
        break;
    case CompareOp::NotEqual:
        compareFunc = GL_NOTEQUAL;
        break;
    case CompareOp::GreaterOrEqual:
        compareFunc = GL_GEQUAL;
        break;
    case CompareOp::Always:
        compareFunc = GL_ALWAYS;
        break;
    }
    return compareFunc;
}

GLenum DepthStencilStateGl::getStencilOperationGl(StencilOp stencilOp)
{
    GLenum stencilOperation{};
    switch (stencilOp)
    {
    case StencilOp::Keep:
        stencilOperation = GL_KEEP;
        break;
    case StencilOp::Zero:
        stencilOperation = GL_ZERO;
        break;
    case StencilOp::Replace:
        stencilOperation = GL_REPLACE;
        break;
    case StencilOp::IncrementAndClamp:
        stencilOperation = GL_INCR;
        break;
    case StencilOp::DecrementAndClamp:
        stencilOperation = GL_DECR;
        break;
    case StencilOp::Invert:
        stencilOperation = GL_INVERT;
        break;
    case StencilOp::IncrementAndWrap:
        stencilOperation = GL_INCR_WRAP;
        break;
    case StencilOp::DecrementAndWrap:
        stencilOperation = GL_DECR_WRAP;
        break;
    }
    return stencilOperation;
}

DepthStencilStateGl::~DepthStencilStateGl() = default;
} // namespace backend