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
}

} // namespace backend