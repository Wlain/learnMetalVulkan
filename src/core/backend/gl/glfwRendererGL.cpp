//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererGL.h"

#include "pipelineGl.h"

#include <GLFW/glfw3.h>
namespace backend
{
GLFWRendererGL::~GLFWRendererGL() = default;

void GLFWRendererGL::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

void GLFWRendererGL::setPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    auto pipelineGL = std::dynamic_pointer_cast<PipelineGL>(pipeline);
    glUseProgram(pipelineGL->program());
    glBindVertexArray(pipelineGL->vao());
    for (const auto& image : pipelineGL->imageInfos())
    {
        const auto& texture = image.second;
        // bind Texture
        glActiveTexture(GL_TEXTURE0 + image.first);
        glBindTexture(texture.target, texture.texture);
    }
    auto depthStencilState = pipelineGL->depthStencilState();
    if (depthStencilState)
    {
        if (depthStencilState->depthTestEnable())
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        glDepthMask(depthStencilState->depthWriteEnable() ? GL_TRUE : GL_FALSE);
        glDepthFunc(DepthStencilStateGl::getCompareOpGl(depthStencilState->depthCompareOp()));
        if (depthStencilState->stencilTestEnable())
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
        glStencilOp(DepthStencilStateGl::getStencilOperationGl(depthStencilState->front().stencilFailOp), DepthStencilStateGl::getStencilOperationGl(depthStencilState->front().depthFailOp), DepthStencilStateGl::getStencilOperationGl(depthStencilState->front().depthStencilPassOp));
        glStencilFunc(DepthStencilStateGl::getCompareOpGl(depthStencilState->front().stencilCompareOp), depthStencilState->front().reference, depthStencilState->front().compareMask);
        glStencilMask(depthStencilState->front().writeMask);
    }
}

} // namespace backend