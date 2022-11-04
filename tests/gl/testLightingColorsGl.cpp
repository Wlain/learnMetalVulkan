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
#include "utils/utils.h"

using namespace backend;
namespace
{
class TestLightingColorsGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestLightingColorsGl() override = default;

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
        vertSource = getFileContents("shaders/colors.vert");
        fragShader = getFileContents("shaders/colors.frag");
        m_pipelineColor = MAKE_SHARED(m_pipelineColor, m_render->device());
        m_pipelineColor->setProgram(vertSource, fragShader);
    }

    void buildBuffers()
    {
        m_vertUniformBuffer = MAKE_SHARED(m_vertUniformBuffer, m_render->device());
        m_vertUniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_fragUniformBuffer = MAKE_SHARED(m_fragUniformBuffer, m_render->device());
        m_fragUniformBuffer->create(sizeof(FragLightingColorUBO), &g_lightingColorUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        auto lightCubeProgram = m_pipelineLightCube->program();
        auto colorProgram = m_pipelineColor->program();
        auto lightCubeVertUboIndex = glGetUniformBlockIndex(lightCubeProgram, "VertMVPMatrixUBO");
        auto colorVertUboIndex = glGetUniformBlockIndex(colorProgram, "VertMVPMatrixUBO");
        auto colorFragUboIndex = glGetUniformBlockIndex(colorProgram, "FragUniformBufferObject");
        glUniformBlockBinding(lightCubeProgram, lightCubeVertUboIndex, g_mvpMatrixUboBinding);
        glUniformBlockBinding(colorProgram, colorVertUboIndex, g_mvpMatrixUboBinding);
        glUniformBlockBinding(colorProgram, colorFragUboIndex, g_lightingColorUboBinding);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_vertUniformBuffer->bufferType(), g_mvpMatrixUboBinding, m_vertUniformBuffer->buffer(), 0, sizeof(VertMVPMatrixUBO));
        glBindBufferRange(m_fragUniformBuffer->bufferType(), g_lightingColorUboBinding, m_fragUniformBuffer->buffer(), 0, sizeof(FragLightingColorUBO));
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_cubeVertices.size() * sizeof(g_cubeVertices[0]), (void*)g_cubeVertices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_pipelineColor->setAttributeDescription(getOneElemAttributesDescriptions());
        m_pipelineLightCube->setAttributeDescription(getOneElemAttributesDescriptions());
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
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_lightPos);
        g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertices.size()));

        // draw lightCube
        m_render->setPipeline(m_pipelineColor);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertices.size()));
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_pipelineLightCube;
    std::shared_ptr<PipelineGL> m_pipelineColor;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_vertUniformBuffer;
    std::shared_ptr<BufferGL> m_fragUniformBuffer;
};
} // namespace

void testLightingColorsGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Lighting Colors" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestLightingColorsGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}