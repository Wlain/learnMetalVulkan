//
// Created by cwb on 2022/9/22.
//

#include "../mesh/globalMeshs.h"
#include "bufferGL.h"
#include "deviceGL.h"
#include "effectBase.h"
#include "engine.h"
#include "glCommonDefine.h"
#include "glfwRendererGL.h"
#include "pipelineGl.h"
#include "textureGl.h"
#include "utils/utils.h"

using namespace backend;

class TestQuadGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestQuadGl() override = default;

    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
        buildTexture();
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
        m_vertexBuffer->create(g_quadVertex.size() * sizeof(g_quadVertex[0]), (void*)g_quadVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_indexBuffer = MAKE_SHARED(m_indexBuffer, m_render->device());
        m_indexBuffer->create(g_quadIndices.size() * sizeof(g_quadIndices[0]), (void*)g_quadIndices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::IndexBuffer);
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
        glDrawElements(GL_TRIANGLES, g_quadIndices.size(), GL_UNSIGNED_SHORT, 0);
    }

private:
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_texture;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_indexBuffer;
};

void testQuadEboGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 640, 480, "OpenGL Example Quad use EBO" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestQuadGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}