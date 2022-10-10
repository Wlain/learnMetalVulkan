//
// Created by cwb on 2022/9/22.
//

#include "textureGl.h"

#include "../mesh/globalMeshs.h"
#include "bufferGL.h"
#include "deviceGL.h"
#include "effectBase.h"
#include "engine.h"
#include "glCommonDefine.h"
#include "glfwRendererGL.h"
#include "pipelineGl.h"
#include "utils/utils.h"

using namespace backend;

class TextureGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TextureGl() override
    {
        glDeleteVertexArrays(1, &m_vao);
    }
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
        buildTexture();
    }
    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/texture.vert");
        std::string fragShader = getFileContents("shaders/texture.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_render->device());
        m_pipeline->setProgram(vertSource, fragShader);
    }
    void buildBuffers()
    {
        auto program = m_pipeline->program();
        auto uboIndex = glGetUniformBlockIndex(program, "UniformBufferObject");
        glUniformBlockBinding(program, uboIndex, 0);
        m_uniformBuffer = MAKE_SHARED(m_uniformBuffer, m_render->device());
        m_uniformBuffer->create(sizeof(UniformBufferObject), &g_mvpMatrix, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_uniformBuffer->bufferType(), uboIndex, m_uniformBuffer->buffer(), 0, sizeof(UniformBufferObject));
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_quadVertex.size() * sizeof(g_quadVertex[0]), (void*)g_quadVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_indexBuffer = MAKE_SHARED(m_indexBuffer, m_render->device());
        m_indexBuffer->create(g_quadIndices.size() * sizeof(g_quadIndices[0]), (void*)g_quadIndices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::IndexBuffer);
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)nullptr);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_render->device());
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void render() override
    {
        static float red = 1.0f;
        //        red = red > 1.0 ? 0.0f : red + 0.01f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        m_render->setPipeline(m_pipeline);
        // bind Texture
        glBindTexture(GL_TEXTURE_2D, m_texture->handle());
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, g_quadIndices.size(), GL_UNSIGNED_SHORT, 0);
    }

private:
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_texture;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_indexBuffer;
    std::shared_ptr<BufferGL> m_uniformBuffer;
    GLuint m_vao{ 0 };
};

void textureGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 480, 480, "OpenGL Example texture" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TextureGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}