//
// Created by william on 2022/11/3.
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
class TestMaterialsGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestMaterialsGl() override
    {
        glDeleteVertexArrays(1, &m_lightSphereVao);
        glDeleteVertexArrays(1, &m_materialCubeVao);
    }
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
    }
    void buildPipeline()
    {
        // lightSphere
        std::string vertSource = getFileContents("shaders/lightColorCube.vert");
        std::string fragShader = getFileContents("shaders/lightColorCube.frag");
        m_lightSpherePipeline = MAKE_SHARED(m_lightSpherePipeline, m_render->device());
        m_lightSpherePipeline->setProgram(vertSource, fragShader);
        // material
        vertSource = getFileContents("shaders/materials.vert");
        fragShader = getFileContents("shaders/materials.frag");
        m_materialCubePipeline = MAKE_SHARED(m_materialCubePipeline, m_render->device());
        m_materialCubePipeline->setProgram(vertSource, fragShader);
    }
    void buildBuffers()
    {
        m_lightSphereVertUbo = MAKE_SHARED(m_lightSphereVertUbo, m_render->device());
        m_lightSphereVertUbo->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_materialCubeFragUbo = MAKE_SHARED(m_materialCubeFragUbo, m_render->device());
        m_materialCubeFragUbo->create(sizeof(FragMaterialsColorUBO), &g_fragMaterialsColorUBO, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_lightSphereFragUbo = MAKE_SHARED(m_lightSphereFragUbo, m_render->device());
        m_lightSphereFragUbo->create(sizeof(FragLightColorUBO), &g_lightColorUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        auto lightSphereProgram = m_lightSpherePipeline->program();
        auto colorProgram = m_materialCubePipeline->program();
        auto lightSphereVertUboIndex = glGetUniformBlockIndex(lightSphereProgram, "VertMVPMatrixUBO");
        auto lightSphereFragUboIndex = glGetUniformBlockIndex(lightSphereProgram, "FragUniformBufferObject");
        auto colorVertUboIndex = glGetUniformBlockIndex(colorProgram, "VertMVPMatrixUBO");
        auto colorFragUboIndex = glGetUniformBlockIndex(colorProgram, "FragUniformBufferObject");
        glUniformBlockBinding(lightSphereProgram, lightSphereVertUboIndex, g_mvpMatrixUboBinding);
        glUniformBlockBinding(lightSphereProgram, lightSphereFragUboIndex, g_lightUboBinding);
        glUniformBlockBinding(colorProgram, colorVertUboIndex, g_mvpMatrixUboBinding);
        glUniformBlockBinding(colorProgram, colorFragUboIndex, g_materialsUboBinding);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(m_lightSphereVertUbo->bufferType(), g_mvpMatrixUboBinding, m_lightSphereVertUbo->buffer(), 0, sizeof(VertMVPMatrixUBO));
        glBindBufferRange(m_lightSphereFragUbo->bufferType(), g_lightUboBinding, m_lightSphereFragUbo->buffer(), 0, sizeof(FragLightColorUBO));
        glBindBufferRange(m_materialCubeFragUbo->bufferType(), g_materialsUboBinding, m_materialCubeFragUbo->buffer(), 0, sizeof(FragMaterialsColorUBO));

        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_materialCubeVertexBuffer = MAKE_SHARED(m_materialCubeVertexBuffer, m_render->device());
        m_materialCubeVertexBuffer->create(g_cubeVerticesWithNormal.size() * sizeof(g_cubeVerticesWithNormal[0]), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_lightSphereVertexBuffer = MAKE_SHARED(m_lightSphereVertexBuffer, m_render->device());
        m_lightSphereVertexBuffer->create(g_sphereMesh.size() * sizeof(g_sphereMesh[0]), (void*)g_sphereMesh.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &m_lightSphereVao);
        glBindVertexArray(m_lightSphereVao);
        glBindBuffer(m_lightSphereVertexBuffer->bufferType(), m_lightSphereVertexBuffer->buffer());
        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1, &m_materialCubeVao);
        glBindVertexArray(m_materialCubeVao);
        glBindBuffer(m_materialCubeVertexBuffer->bufferType(), m_materialCubeVertexBuffer->buffer());
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
        // draw lighting source
        m_render->setPipeline(m_lightSpherePipeline);
        glBindVertexArray(m_lightSphereVao);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_lightColorUbo.lightColor.x = static_cast<float>(sin(m_duringTime * 2.0));
        g_lightColorUbo.lightColor.y = static_cast<float>(sin(m_duringTime * 0.7));
        g_lightColorUbo.lightColor.z = static_cast<float>(sin(m_duringTime * 1.3));
        m_lightSphereFragUbo->update(&g_lightColorUbo, sizeof(FragLightColorUBO), 0);
        glm::vec4 diffuse = g_lightColorUbo.lightColor * glm::vec4(0.5f); // decrease the influence
        glm::vec4 ambient = diffuse * glm::vec4(0.2f);                    // decrease the influence
        glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };
        g_fragMaterialsColorUBO.light.position.x = 1.0f + sin(m_duringTime) * 2.0f;
        g_fragMaterialsColorUBO.light.position.y = sin(m_duringTime / 2.0f) * 1.0f;
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_fragMaterialsColorUBO.light.position));
        g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.05f)); // a smaller cube
        m_lightSphereVertUbo->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_sphereMesh.size()));

        // draw lightSphere
        m_render->setPipeline(m_materialCubePipeline);
        glBindVertexArray(m_materialCubeVao);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        m_lightSphereVertUbo->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        g_fragMaterialsColorUBO.viewPos = glm::vec4(m_camera.position, 1.0f);
        g_fragMaterialsColorUBO.light.ambient = ambient;
        g_fragMaterialsColorUBO.light.diffuse = diffuse;
        g_fragMaterialsColorUBO.light.specular = specular;
        m_materialCubeFragUbo->update(&g_fragMaterialsColorUBO, sizeof(FragMaterialsColorUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormal.size()));
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_lightSpherePipeline;
    std::shared_ptr<PipelineGL> m_materialCubePipeline;
    std::shared_ptr<BufferGL> m_materialCubeVertexBuffer;
    std::shared_ptr<BufferGL> m_lightSphereVertexBuffer;
    std::shared_ptr<BufferGL> m_lightSphereVertUbo;
    std::shared_ptr<BufferGL> m_materialCubeFragUbo;
    std::shared_ptr<BufferGL> m_lightSphereFragUbo;
    GLuint m_lightSphereVao{ 0 };
    GLuint m_materialCubeVao{ 0 };
};
} // namespace

void testMaterialsGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Basic Lighting Diffuse" };
    DeviceGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestMaterialsGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}