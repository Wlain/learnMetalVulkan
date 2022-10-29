//
// Created by william on 2022/10/21.
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
namespace
{
class TestCubeMultipleGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestCubeMultipleGl() override
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
        g_mvpMatrix.view = glm::translate(g_mvpMatrix.view, glm::vec3(0.0f, 0.0f, -3.0f));
        m_uniformBuffer->create(sizeof(UniformBufferObject), &g_mvpMatrix, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_uniformBuffer->bufferType(), uboIndex, m_uniformBuffer->buffer(), 0, sizeof(UniformBufferObject));
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_cubeVertex.size() * sizeof(g_cubeVertex[0]), (void*)g_cubeVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)nullptr);
        glEnableVertexAttribArray(0);
        // texCoord attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_render->device());
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void resize(int width, int height) override
    {
        g_mvpMatrix.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }

    void render() override
    {
        glEnable(GL_DEPTH_TEST);
        static float red = 1.0f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_render->setPipeline(m_pipeline);
        // bind Texture
        glBindTexture(GL_TEXTURE_2D, m_texture->handle());
        glBindVertexArray(m_vao);
        for (unsigned int i = 0; i < g_cubePositions.size(); i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrix.model = glm::mat4(1.0f);
            g_mvpMatrix.model = glm::translate(g_mvpMatrix.model, g_cubePositions[i]);
            g_mvpMatrix.model = glm::rotate(g_mvpMatrix.model, m_duringTime, glm::vec3(0.5f, 1.0f, 0.0f));
            float angle = 20.0f * i;
            g_mvpMatrix.model = glm::rotate(g_mvpMatrix.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            m_uniformBuffer->update(&g_mvpMatrix, sizeof(UniformBufferObject), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<uint32_t>(g_cubeVertex.size()));
        }
    }

private:
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_texture;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_uniformBuffer;
    GLuint m_vao{ 0 };
};
} // namespace

void testCubeMultipleGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Cube Multiple" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestCubeMultipleGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}