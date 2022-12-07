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

GLenum DepthStencilStateGL::getCompareOpGl(CompareOp op)
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

DepthStencilStateGL::~DepthStencilStateGL() = default;
} // namespace backend