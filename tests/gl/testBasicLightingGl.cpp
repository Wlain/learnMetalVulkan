//
// Created by william on 2022/11/2.
//

#include "../mesh/globalMeshs.h"
#include "bufferGl.h"
#include "depthStencilStateGl.h"
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
class TestBasicLightingGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestBasicLightingGl() override = default;
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
        buildDepthStencilStates();
        buildDescriptorsSets();
    }

    void buildPipeline()
    {
        // lightCube
        std::string vertSource = getFileContents("shaders/lightCube.vert");
        std::string fragShader = getFileContents("shaders/lightCube.frag");
        m_lightCubePipeline = MAKE_SHARED(m_lightCubePipeline, m_render->device());
        m_lightCubePipeline->setProgram(vertSource, fragShader);
        // color
        vertSource = getFileContents("shaders/basicLighting.vert");
        fragShader = getFileContents("shaders/basicLighting.frag");
        m_basicLightingPipelineLight = MAKE_SHARED(m_basicLightingPipelineLight, m_render->device());
        m_basicLightingPipelineLight->setProgram(vertSource, fragShader);
    }

    void buildBuffers()
    {
        m_vertUniformBuffer = MAKE_SHARED(m_vertUniformBuffer, m_render->device());
        m_vertUniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_fragUniformBuffer = MAKE_SHARED(m_fragUniformBuffer, m_render->device());
        m_fragUniformBuffer->create(sizeof(FragBasicLightingColorUBO), &g_basicLightingColorUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        m_lightingCubeVertexBuffer = MAKE_SHARED(m_lightingCubeVertexBuffer, m_render->device());
        m_lightingCubeVertexBuffer->create(g_cubeVertices.size() * sizeof(g_cubeVertices[0]), (void*)g_cubeVertices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_lightCubePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_materialVertexBuffer = MAKE_SHARED(m_materialVertexBuffer, m_render->device());
        m_materialVertexBuffer->create(g_cubeVerticesWithNormal.size() * sizeof(g_cubeVerticesWithNormal[0]), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_basicLightingPipelineLight->setAttributeDescription(getTwoElemsAttributesDescriptions());
    }

    void buildDepthStencilStates()
    {
        m_depthStencilState = MAKE_SHARED(m_depthStencilState, m_render->device());
        m_depthStencilState->setDepthCompareOp(CompareOp::Less);
        m_depthStencilState->setDepthTestEnable(true);
        m_depthStencilState->setDepthWriteEnable(true);
        m_lightCubePipeline->setDepthStencilState(m_depthStencilState);
        m_basicLightingPipelineLight->setDepthStencilState(m_depthStencilState);
    }

    void buildDescriptorsSets()
    {
        std::map<int32_t, DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_vertUniformBuffer->bufferType(),
                                         .buffer = m_vertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        m_lightCubeDescriptorSet = MAKE_SHARED(m_lightCubeDescriptorSet, m_render->device());
        m_lightCubeDescriptorSet->createDescriptorSetLayout(g_lightCubeShaderResource);
        m_lightCubeDescriptorSet->createDescriptorSets(bufferInfos, {});
        m_basicLightingPipelineLight->setDescriptorSet(m_lightCubeDescriptorSet);

        bufferInfos = {
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_vertUniformBuffer->bufferType(),
                                         .buffer = m_vertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_lightingColorUboBinding, DescriptorBufferInfo{ .bufferType = m_fragUniformBuffer->bufferType(), .buffer = m_fragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragBasicLightingColorUBO) } }
        };
        m_basicLightingDescriptorSet = MAKE_SHARED(m_basicLightingDescriptorSet, m_render->device());
        m_basicLightingDescriptorSet->createDescriptorSetLayout(g_basicLightingShaderResource);
        m_basicLightingDescriptorSet->createDescriptorSets(bufferInfos, {});
        m_basicLightingPipelineLight->setDescriptorSet(m_basicLightingDescriptorSet);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void render() override
    {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // draw lightCube
        m_render->setPipeline(m_lightCubePipeline);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_basicLightingColorUbo.lightPos.x = 1.0f + sin(m_duringTime) * 2.0f;
        g_basicLightingColorUbo.lightPos.y = sin(m_duringTime / 2.0f) * 1.0f;
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_basicLightingColorUbo.lightPos));
        g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormal.size()));

        // draw basic lighting cube
        m_render->setPipeline(m_basicLightingPipelineLight);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        g_basicLightingColorUbo.viewPos = glm::vec4(m_camera.position, 1.0f);
        m_fragUniformBuffer->update(&g_basicLightingColorUbo, sizeof(FragBasicLightingColorUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVerticesWithNormal.size()));
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_lightCubePipeline;
    std::shared_ptr<PipelineGL> m_basicLightingPipelineLight;
    std::shared_ptr<BufferGL> m_lightingCubeVertexBuffer;
    std::shared_ptr<BufferGL> m_materialVertexBuffer;
    std::shared_ptr<BufferGL> m_vertUniformBuffer;
    std::shared_ptr<BufferGL> m_fragUniformBuffer;
    std::shared_ptr<DescriptorSetGl> m_lightCubeDescriptorSet;
    std::shared_ptr<DescriptorSetGl> m_basicLightingDescriptorSet;
    std::shared_ptr<DepthStencilStateGl> m_depthStencilState;
};
} // namespace

void testBasicLightingGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Basic Lighting basicLighting" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestBasicLightingGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}