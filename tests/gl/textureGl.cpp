//
// Created by cwb on 2022/9/22.
//

#include "textureGl.h"

#include "../mesh/globalMeshs.h"
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
        glDeleteBuffers(1, &m_vbo);
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
        std::string vertSource = getFileContents("shaders/checkTexCoord.vert");
        std::string fragShader = getFileContents("shaders/checkTexCoord.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_render->device());
        m_pipeline->setProgram(vertSource, fragShader);
    }
    void buildBuffers()
    {
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quadVertex[0]) * g_quadVertex.size(), g_quadVertex.data(), GL_STATIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_render->device());
        m_texture->createWithFileName("textures/test.jpg", true);
        LOG_ERROR("textureID: {}", m_texture->handle());
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
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

private:
    GLuint m_vao{ 0 };
    GLuint m_vbo{ 0 };
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_texture;
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