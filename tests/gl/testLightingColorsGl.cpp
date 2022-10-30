//
// Created by william on 2022/10/29.
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
class TestLightingColorsGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestLightingColorsGl() override
    {
        glDeleteVertexArrays(1, &m_vao);
    }
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
    }
    void buildPipeline()
    {
        // lightCube
        std::string vertSource = getFileContents("shaders/lightCube.vert");
        std::string fragShader = getFileContents("shaders/lightCube.frag");
        m_pipelineLightCube = MAKE_SHARED(m_pipelineLightCube, m_render->device());
        m_pipelineLightCube->setProgram(vertSource, fragShader);
        // color
        vertSource = getFileContents("shaders/color.vert");
        fragShader = getFileContents("shaders/color.frag");
        m_pipelineColor = MAKE_SHARED(m_pipelineColor, m_render->device());
        m_pipelineColor->setProgram(vertSource, fragShader);
    }
    void buildBuffers()
    {
        m_vertUniformBuffer = MAKE_SHARED(m_vertUniformBuffer, m_render->device());
        m_vertUniformBuffer->create(sizeof(UniformBufferObject), &m_vertUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        auto program = m_pipelineLightCube->program();
        glUseProgram(program);
        auto uboIndex = glGetUniformBlockIndex(program, "UniformBufferObject");
        glUniformBlockBinding(program, uboIndex, 0);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_vertUniformBuffer->bufferType(), uboIndex, m_vertUniformBuffer->buffer(), 0, sizeof(UniformBufferObject));

        program = m_pipelineColor->program();
        glUseProgram(program);
        uboIndex = glGetUniformBlockIndex(program, "VertUniformBufferObject");
        glUniformBlockBinding(program, uboIndex, 0);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_vertUniformBuffer->bufferType(), uboIndex, m_vertUniformBuffer->buffer(), 0, sizeof(UniformBufferObject));
        uboIndex = glGetUniformBlockIndex(program, "FragUniformBufferObject");
        glUniformBlockBinding(program, uboIndex, 0);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_cubeVertices.size() * sizeof(g_cubeVertices[0]), (void*)g_cubeVertices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)nullptr);
        glEnableVertexAttribArray(0);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        m_vertUbo.view = m_camera.viewMatrix();
        m_vertUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void render() override
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // draw color
        m_render->setPipeline(m_pipelineLightCube);
        glBindVertexArray(m_vao);
        // calculate the model matrix for each object and pass it to shader before drawing
        m_vertUbo.model = glm::mat4(1.0f);
        m_vertUbo.model = glm::translate(m_vertUbo.model, s_lightPos);
        m_vertUbo.model = glm::scale(m_vertUbo.model, glm::vec3(0.2f)); // a smaller cube
        m_vertUniformBuffer->update(&m_vertUbo, sizeof(UniformBufferObject), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertices.size()));

        // draw lightCube
        m_render->setPipeline(m_pipelineColor);
        // calculate the model matrix for each object and pass it to shader before drawing
        m_vertUbo.model = glm::mat4(1.0f);
        m_vertUniformBuffer->update(&m_vertUbo, sizeof(UniformBufferObject), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertices.size()));
    }

public:
    // lighting
    inline constexpr static glm::vec3 s_lightPos{ 1.2f, 1.0f, 2.0f };

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_pipelineLightCube;
    std::shared_ptr<PipelineGL> m_pipelineColor;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_vertUniformBuffer;
    std::shared_ptr<BufferGL> m_fragUniformBuffer;
    UniformBufferObject m_vertUbo;
    GLuint m_vao{ 0 };
};
} // namespace

void testLightingColorsGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Lighting Colors" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestLightingColorsGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}