//
// Created by william on 2022/11/2.
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
namespace
{
class TestBasicLightingDiffuseGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestBasicLightingDiffuseGl() override
    {
        glDeleteVertexArrays(1, &m_cubeVao);
        glDeleteVertexArrays(1, &m_lightCubeVao);
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
        vertSource = getFileContents("shaders/basicLighting.vert");
        fragShader = getFileContents("shaders/basicLighting.frag");
        m_pipelineLightDiffuse = MAKE_SHARED(m_pipelineLightDiffuse, m_render->device());
        m_pipelineLightDiffuse->setProgram(vertSource, fragShader);
    }
    void buildBuffers()
    {
        m_vertUniformBuffer = MAKE_SHARED(m_vertUniformBuffer, m_render->device());
        m_vertUniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_fragUniformBuffer = MAKE_SHARED(m_fragUniformBuffer, m_render->device());
        m_fragUniformBuffer->create(sizeof(FragBasicLightingColorUBO), &g_basicLightingColorUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        auto lightCubeProgram = m_pipelineLightCube->program();
        auto colorProgram = m_pipelineLightDiffuse->program();
        auto lightCubeVertUboIndex = glGetUniformBlockIndex(lightCubeProgram, "VertMVPMatrixUBO");
        auto colorVertUboIndex = glGetUniformBlockIndex(colorProgram, "VertMVPMatrixUBO");
        auto colorFragUboIndex = glGetUniformBlockIndex(colorProgram, "FragUniformBufferObject");
        glUniformBlockBinding(lightCubeProgram, lightCubeVertUboIndex, g_mvpMatrixUboBinding);
        glUniformBlockBinding(colorProgram, colorVertUboIndex, g_mvpMatrixUboBinding);
        glUniformBlockBinding(colorProgram, colorFragUboIndex, g_basicLightingColorUboBinding);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_vertUniformBuffer->bufferType(), g_mvpMatrixUboBinding, m_vertUniformBuffer->buffer(), 0, sizeof(VertMVPMatrixUBO));
        glBindBufferRange(m_fragUniformBuffer->bufferType(), g_basicLightingColorUboBinding, m_fragUniformBuffer->buffer(), 0, sizeof(FragBasicLightingColorUBO));

        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_cubeVerticesWithNormal.size() * sizeof(g_cubeVerticesWithNormal[0]), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &m_cubeVao);
        glBindVertexArray(m_cubeVao);
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(LightingVertex), (void*)offsetof(LightingVertex, position));
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1, &m_lightCubeVao);
        glBindVertexArray(m_lightCubeVao);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(LightingVertex), (void*)offsetof(LightingVertex, position));
        glEnableVertexAttribArray(0);
        // normal attribute
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LightingVertex), (void*)offsetof(LightingVertex, normal));
        glEnableVertexAttribArray(1);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void render() override
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // draw color
        m_render->setPipeline(m_pipelineLightCube);
        glBindVertexArray(m_cubeVao);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_lightPos);
        g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormal.size()));

        // draw lightCube
        m_render->setPipeline(m_pipelineLightDiffuse);
        glBindVertexArray(m_lightCubeVao);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        g_basicLightingColorUbo.viewPos = glm::vec4(m_camera.position, 1.0f);
        m_fragUniformBuffer->update(&g_basicLightingColorUbo, sizeof(FragBasicLightingColorUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormal.size()));
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_pipelineLightCube;
    std::shared_ptr<PipelineGL> m_pipelineLightDiffuse;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_vertUniformBuffer;
    std::shared_ptr<BufferGL> m_fragUniformBuffer;
    GLuint m_cubeVao{ 0 };
    GLuint m_lightCubeVao{ 0 };
};
} // namespace

void testBasicLightingDiffuseGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Basic Lighting Diffuse" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestBasicLightingDiffuseGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}