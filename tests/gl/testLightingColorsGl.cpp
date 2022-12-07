//
// Created by william on 2022/10/29.
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
        // colors
        vertSource = getFileContents("shaders/colors.vert");
        fragShader = getFileContents("shaders/colors.frag");
        m_colorsPipeline = MAKE_SHARED(m_colorsPipeline, m_render->device());
        m_colorsPipeline->setProgram(vertSource, fragShader);
    }

    void buildBuffers()
    {
        m_vertUniformBuffer = MAKE_SHARED(m_vertUniformBuffer, m_render->device());
        m_vertUniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_fragUniformBuffer = MAKE_SHARED(m_fragUniformBuffer, m_render->device());
        m_fragUniformBuffer->create(sizeof(FragLightingColorsUBO), &g_lightingColorsUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_cubeVertices.size() * sizeof(g_cubeVertices[0]), (void*)g_cubeVertices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_colorsPipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_lightCubePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
    }

    void buildDepthStencilStates()
    {
        m_depthStencilState = MAKE_SHARED(m_depthStencilState, m_render->device());
        m_depthStencilState->setDepthCompareOp(CompareOp::Less);
        m_depthStencilState->setDepthTestEnable(true);
        m_depthStencilState->setDepthWriteEnable(true);
        m_colorsPipeline->setDepthStencilState(m_depthStencilState);
        m_lightCubePipeline->setDepthStencilState(m_depthStencilState);
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
        m_lightCubePipeline->setDescriptorSet(m_lightCubeDescriptorSet);
        bufferInfos = {
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_vertUniformBuffer->bufferType(),
                                         .buffer = m_vertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_lightingColorUboBinding, DescriptorBufferInfo{ .bufferType = m_fragUniformBuffer->bufferType(), .buffer = m_fragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragLightingColorsUBO) } }
        };
        m_colorsDescriptorSet = MAKE_SHARED(m_colorsDescriptorSet, m_render->device());
        m_colorsDescriptorSet->createDescriptorSetLayout(g_basicLightingShaderResource);
        m_colorsDescriptorSet->createDescriptorSets(bufferInfos, {});
        m_colorsPipeline->setDescriptorSet(m_colorsDescriptorSet);
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
        // draw colors
        m_render->setPipeline(m_lightCubePipeline);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_lightPos);
        g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertices.size()));

        // draw lightCube
        m_render->setPipeline(m_colorsPipeline);
        // calculate the model matrix for each object and pass it to shader before drawing
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertices.size()));
    }

private:
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<PipelineGL> m_lightCubePipeline;
    std::shared_ptr<PipelineGL> m_colorsPipeline;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_vertUniformBuffer;
    std::shared_ptr<BufferGL> m_fragUniformBuffer;
    std::shared_ptr<DescriptorSetGl> m_lightCubeDescriptorSet;
    std::shared_ptr<DescriptorSetGl> m_colorsDescriptorSet;
    std::shared_ptr<DepthStencilStateGL> m_depthStencilState;
};
} // namespace

void testLightingColorsGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Lighting colors" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestLightingColorsGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}