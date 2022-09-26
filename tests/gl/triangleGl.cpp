//
// Created by cwb on 2022/9/8.
//

#include "../mesh/globalMeshs.h"
#include "deviceGL.h"
#include "effectBase.h"
#include "engine.h"
#include "glCommonDefine.h"
#include "glfwRendererGL.h"
#include "pipelineGl.h"
#include "utils/utils.h"

using namespace backend;

class TriangleGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TriangleGl() override
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }
    void initialize() override
    {
        m_render = static_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
    }
    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangleVertex[0]) * g_triangleVertex.size(), g_triangleVertex.data(), GL_STATIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)nullptr);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void render() override
    {
        static float red = 1.0f;
        red = red > 1.0 ? 0.0f : red + 0.01f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        m_render->setPipeline(m_pipeline);
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

private:
    GLuint m_vao{ 0 };
    GLuint m_vbo{ 0 };
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
};

void triangleGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 640, 480, "OpenGL Example triangle" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TriangleGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}
