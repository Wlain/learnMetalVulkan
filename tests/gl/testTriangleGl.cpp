//
// Created by cwb on 2022/9/8.
//

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

class TestTriangleGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestTriangleGl() override = default;

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
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_triangleVertex.size() * sizeof(g_triangleVertex[0]), (void*)g_triangleVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
    }

    void render() override
    {
        static float red = 1.0f;
        red = red > 1.0 ? 0.0f : red + 0.01f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        m_render->setPipeline(m_pipeline);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

private:
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
};

void testTriangleGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 640, 480, "OpenGL Example triangle" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestTriangleGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}
