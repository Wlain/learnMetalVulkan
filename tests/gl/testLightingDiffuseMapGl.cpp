//
// Created by william on 2022/12/2.
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
#include "textureGl.h"

using namespace backend;
namespace
{
class TestLightingDiffuseMapGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestLightingDiffuseMapGl() override = default;
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildTexture();
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
        // lighting Diffuse Map
        vertSource = getFileContents("shaders/lightingDiffuseMap.vert");
        fragShader = getFileContents("shaders/lightingDiffuseMap.frag");
        m_lightingDiffuseMapCubePipeline = MAKE_SHARED(m_lightingDiffuseMapCubePipeline, m_render->device());
        m_lightingDiffuseMapCubePipeline->setProgram(vertSource, fragShader);
    }

    void buildTexture()
    {
        m_diffuseMapTexture = MAKE_SHARED(m_diffuseMapTexture, m_render->device());
        m_diffuseMapTexture->createWithFileName("textures/test.jpg", true);
        m_specularMapTexture = MAKE_SHARED(m_specularMapTexture, m_render->device());
        m_specularMapTexture->createWithFileName("textures/container2_specular.png", true);
    }

    void buildBuffers()
    {
        m_commonVertUbo = MAKE_SHARED(m_commonVertUbo, m_render->device());
        m_commonVertUbo->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_lightingDiffuseMapCubeFragUbo = MAKE_SHARED(m_lightingDiffuseMapCubeFragUbo, m_render->device());
        m_lightingDiffuseMapCubeFragUbo->create(sizeof(FragMaterialsColorUBO), &g_fragMaterialsColorUBO, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_lightSphereFragUbo = MAKE_SHARED(m_lightSphereFragUbo, m_render->device());
        m_lightSphereFragUbo->create(sizeof(FragLightSphereUBO), &g_lightColorUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        /// UBO
        m_lightSphereVertexBuffer = MAKE_SHARED(m_lightSphereVertexBuffer, m_render->device());
        m_lightSphereVertexBuffer->create(g_sphereMesh.size() * sizeof(g_sphereMesh[0]), (void*)g_sphereMesh.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_lightSpherePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_lightingDiffuseMapCubeVertexBuffer = MAKE_SHARED(m_lightingDiffuseMapCubeVertexBuffer, m_render->device());
        m_lightingDiffuseMapCubeVertexBuffer->create(g_cubeVerticesWithNormalTexCoord.size() * sizeof(g_cubeVerticesWithNormalTexCoord[0]), (void*)g_cubeVerticesWithNormalTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_lightingDiffuseMapCubePipeline->setAttributeDescription(getThreeElemsAttributesDescriptions());
    }

    void buildDescriptorsSets()
    {
        std::map<int32_t, DescriptorBufferInfo> bufferInfos{
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
            { g_fragDiffuseMapUboBinding, DescriptorBufferInfo{ .bufferType = m_lightingDiffuseMapCubeFragUbo->bufferType(), .buffer = m_lightingDiffuseMapCubeFragUbo->buffer(), .offset = 0, .range = sizeof(FragDiffuseMapUBO) } }
        };
        std::map<int32_t, DescriptorImageInfo> imageInfos{
            { g_diffuseTextureBinding, DescriptorImageInfo{
                                    .target = m_diffuseMapTexture->target(),
                                    .texture = m_diffuseMapTexture->handle(),
                                    .name = "diffuseTexture"
                                } },
            { g_specularTextureBinding, DescriptorImageInfo{
                                           .target = m_specularMapTexture->target(),
                                           .texture = m_specularMapTexture->handle(),
                                            .name = "specularTexture"
                                       } },
        };
        m_materialsDescriptorSet = MAKE_SHARED(m_materialsDescriptorSet, m_render->device());
        m_materialsDescriptorSet->createDescriptorSetLayout(g_diffuseMapShaderResource);
        m_materialsDescriptorSet->createDescriptorSets(bufferInfos, imageInfos);
        m_lightingDiffuseMapCubePipeline->setDescriptorSet(m_materialsDescriptorSet);
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
            g_fragDiffuseMapUBO.light.position.x = 1.0f + sin(m_duringTime) * 2.0f;
            g_fragDiffuseMapUBO.light.position.y = sin(m_duringTime / 2.0f) * 1.0f;
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_fragDiffuseMapUBO.light.position));
            g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.05f)); // a smaller cube
            m_commonVertUbo->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_sphereMesh.size()));
        }
        // draw diffuse map cube
        {
            m_render->setPipeline(m_lightingDiffuseMapCubePipeline);
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            m_commonVertUbo->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
            g_fragDiffuseMapUBO.viewPos = glm::vec4(m_camera.position, 1.0f);
            g_fragDiffuseMapUBO.light.diffuse = g_lightColorUbo.lightColor * glm::vec4(0.5f);            // decrease the influence;
            g_fragDiffuseMapUBO.light.ambient = g_fragDiffuseMapUBO.light.diffuse * glm::vec4(0.2f); // decrease the influence
            g_fragDiffuseMapUBO.light.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_lightingDiffuseMapCubeFragUbo->update(&g_fragDiffuseMapUBO, sizeof(FragDiffuseMapUBO), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormalTexCoord.size()));
        }
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_lightSpherePipeline;
    std::shared_ptr<PipelineGL> m_lightingDiffuseMapCubePipeline;
    std::shared_ptr<BufferGL> m_lightingDiffuseMapCubeVertexBuffer;
    std::shared_ptr<BufferGL> m_lightSphereVertexBuffer;
    std::shared_ptr<TextureGL> m_diffuseMapTexture;
    std::shared_ptr<TextureGL> m_specularMapTexture;
    std::shared_ptr<BufferGL> m_commonVertUbo;
    std::shared_ptr<BufferGL> m_lightingDiffuseMapCubeFragUbo;
    std::shared_ptr<BufferGL> m_lightSphereFragUbo;
    std::shared_ptr<DescriptorSetGl> m_lightSphereDescriptorSet;
    std::shared_ptr<DescriptorSetGl> m_materialsDescriptorSet;
};
} // namespace

void testLightingDiffuseMapGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Lighting Diffuse Map" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestLightingDiffuseMapGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}