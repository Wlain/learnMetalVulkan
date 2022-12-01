//
// Created by william on 2022/11/3.
//

#include "../mesh/globalMeshs.h"
#include "bufferGl.h"
#include "descriptorSetGl.h"
#include "deviceGl.h"
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
    ~TestMaterialsGl() override = default;
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
        buildDescriptorsSets();
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
        m_commonVertUbo = MAKE_SHARED(m_commonVertUbo, m_render->device());
        m_commonVertUbo->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_materialCubeFragUbo = MAKE_SHARED(m_materialCubeFragUbo, m_render->device());
        m_materialCubeFragUbo->create(sizeof(FragMaterialsColorUBO), &g_fragMaterialsColorUBO, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_lightSphereFragUbo = MAKE_SHARED(m_lightSphereFragUbo, m_render->device());
        m_lightSphereFragUbo->create(sizeof(FragLightSphereUBO), &g_lightColorUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        /// UBO
        m_lightSphereVertexBuffer = MAKE_SHARED(m_lightSphereVertexBuffer, m_render->device());
        m_lightSphereVertexBuffer->create(g_sphereMesh.size() * sizeof(g_sphereMesh[0]), (void*)g_sphereMesh.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_lightSpherePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_materialCubeVertexBuffer = MAKE_SHARED(m_materialCubeVertexBuffer, m_render->device());
        m_materialCubeVertexBuffer->create(g_cubeVerticesWithNormal.size() * sizeof(g_cubeVerticesWithNormal[0]), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_materialCubePipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
    }

    void buildDescriptorsSets()
    {
        std::map<uint32_t, DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_commonVertUbo->bufferType(),
                                         .buffer = m_commonVertUbo->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_lightSphereUboBinding, DescriptorBufferInfo{ .bufferType = m_lightSphereFragUbo->bufferType(), .buffer = m_lightSphereFragUbo->buffer(), .offset = 0, .range = sizeof(FragLightSphereUBO) } },
        };
        m_lightSphereDescriptorSet = MAKE_SHARED(m_lightSphereDescriptorSet, m_render->device());
        m_lightSphereDescriptorSet->createDescriptorSetLayout(g_lightSphereShaderResource);
        m_lightSphereDescriptorSet->createDescriptorSets(bufferInfos, {});
        m_lightSpherePipeline->setDescriptorSet(m_lightSphereDescriptorSet);

        bufferInfos = {
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_commonVertUbo->bufferType(),
                                         .buffer = m_commonVertUbo->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_materialsUboBinding, DescriptorBufferInfo{ .bufferType = m_materialCubeFragUbo->bufferType(), .buffer = m_materialCubeFragUbo->buffer(), .offset = 0, .range = sizeof(FragMaterialsColorUBO) } }
        };
        m_materialsDescriptorSet = MAKE_SHARED(m_materialsDescriptorSet, m_render->device());
        m_materialsDescriptorSet->createDescriptorSetLayout(g_materialsShaderResource);
        m_materialsDescriptorSet->createDescriptorSets(bufferInfos, {});
        m_materialCubePipeline->setDescriptorSet(m_materialsDescriptorSet);
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
        // draw lighting sphere
        {
            m_render->setPipeline(m_lightSpherePipeline);
            // calculate the model matrix for each object and pass it to shader before drawing
            g_lightColorUbo.lightColor.x = static_cast<float>(std::abs(sin(m_duringTime * 2.0)));
            g_lightColorUbo.lightColor.y = static_cast<float>(std::abs(sin(m_duringTime * 0.7)));
            g_lightColorUbo.lightColor.z = static_cast<float>(std::abs(sin(m_duringTime * 1.3)));
            m_lightSphereFragUbo->update(&g_lightColorUbo, sizeof(FragLightSphereUBO), 0);
            g_fragMaterialsColorUBO.light.position.x = 1.0f + sin(m_duringTime) * 2.0f;
            g_fragMaterialsColorUBO.light.position.y = sin(m_duringTime / 2.0f) * 1.0f;
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_fragMaterialsColorUBO.light.position));
            g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.05f)); // a smaller cube
            m_commonVertUbo->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_sphereMesh.size()));
        }
        // draw materials cube
        {
            m_render->setPipeline(m_materialCubePipeline);
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            m_commonVertUbo->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
            g_fragMaterialsColorUBO.viewPos = glm::vec4(m_camera.position, 1.0f);
            g_fragMaterialsColorUBO.light.diffuse = g_lightColorUbo.lightColor * glm::vec4(0.5f);            // decrease the influence;
            g_fragMaterialsColorUBO.light.ambient = g_fragMaterialsColorUBO.light.diffuse * glm::vec4(0.2f); // decrease the influence
            g_fragMaterialsColorUBO.light.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_materialCubeFragUbo->update(&g_fragMaterialsColorUBO, sizeof(FragMaterialsColorUBO), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormal.size()));
        }
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_lightSpherePipeline;
    std::shared_ptr<PipelineGL> m_materialCubePipeline;
    std::shared_ptr<BufferGL> m_materialCubeVertexBuffer;
    std::shared_ptr<BufferGL> m_lightSphereVertexBuffer;
    std::shared_ptr<BufferGL> m_commonVertUbo;
    std::shared_ptr<BufferGL> m_materialCubeFragUbo;
    std::shared_ptr<BufferGL> m_lightSphereFragUbo;
    std::shared_ptr<DescriptorSetGl> m_lightSphereDescriptorSet;
    std::shared_ptr<DescriptorSetGl> m_materialsDescriptorSet;
};
} // namespace

void testMaterialsGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Materials" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestMaterialsGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}