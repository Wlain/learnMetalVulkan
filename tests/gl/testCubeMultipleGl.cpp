//
// Created by william on 2022/10/21.
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
#include "textureGl.h"
#include "utils/utils.h"

using namespace backend;
namespace
{
class TestCubeMultipleGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestCubeMultipleGl() override = default;
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererGL*>(m_renderer);
        buildPipeline();
        buildBuffers();
        buildTexture();
        buildDescriptorsSets();
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
        m_uniformBuffer = MAKE_SHARED(m_uniformBuffer, m_render->device());
        g_mvpMatrixUbo.view = glm::translate(g_mvpMatrixUbo.view, glm::vec3(0.0f, 0.0f, -3.0f));
        m_uniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_cubeVertex.size() * sizeof(g_cubeVertex[0]), (void*)g_cubeVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_render->device());
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void buildDescriptorsSets()
    {
        std::map<int32_t, DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_uniformBuffer->bufferType(),
                                         .buffer = m_uniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        std::map<int32_t, DescriptorImageInfo> imageInfos{
            { g_textureBinding, DescriptorImageInfo{
                                    .target = m_texture->target(),
                                    .texture = m_texture->handle(),
                                } }
        };
        m_descriptorSet = MAKE_SHARED(m_descriptorSet, m_render->device());
        m_descriptorSet->createDescriptorSetLayout(g_textureShaderResource);
        m_descriptorSet->createDescriptorSets(bufferInfos, imageInfos);
        m_pipeline->setDescriptorSet(m_descriptorSet);
    }

    void resize(int width, int height) override
    {
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }

    void render() override
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_render->setPipeline(m_pipeline);
        for (unsigned int i = 0; i < g_cubePositions.size(); i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_cubePositions[i]);
            g_mvpMatrixUbo.model = glm::rotate(g_mvpMatrixUbo.model, m_duringTime, glm::vec3(0.5f, 1.0f, 0.0f));
            float angle = 20.0f * i;
            g_mvpMatrixUbo.model = glm::rotate(g_mvpMatrixUbo.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            m_uniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<uint32_t>(g_cubeVertex.size()));
        }
    }

private:
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_texture;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_uniformBuffer;
    std::shared_ptr<DescriptorSetGl> m_descriptorSet;
};
} // namespace

void testCubeMultipleGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Cube Multiple" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestCubeMultipleGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}