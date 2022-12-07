//
// Created by william on 2022/12/5.
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

class TestDepthTestGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestDepthTestGl() override = default;
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
        std::string vertSource = getFileContents("shaders/texture.vert");
        std::string fragShader = getFileContents("shaders/texture.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_render->device());
        m_pipeline->setProgram(vertSource, fragShader);
    }

    void buildTexture()
    {
        m_cubeTexture = MAKE_SHARED(m_cubeTexture, m_render->device());
        m_cubeTexture->createWithFileName("textures/marble.jpg", true);
        m_planeTexture = MAKE_SHARED(m_cubeTexture, m_render->device());
        m_planeTexture->createWithFileName("textures/metal.png", true);
    }

    void buildBuffers()
    {
        m_uniformBuffer = MAKE_SHARED(m_uniformBuffer, m_render->device());
        m_uniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_cubeVertexBuffer = MAKE_SHARED(m_cubeVertexBuffer, m_render->device());
        m_cubeVertexBuffer->create(g_cubeVerticesPosTexCoord.size() * sizeof(g_cubeVerticesPosTexCoord[0]), (void*)g_cubeVerticesPosTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_planeVertexBuffer = MAKE_SHARED(m_planeVertexBuffer, m_render->device());
        m_planeVertexBuffer->create(g_planeVerticesPosTexCoord.size() * sizeof(g_planeVerticesPosTexCoord[0]), (void*)g_planeVerticesPosTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
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
                                    .name = "inputTexture",
                                    .target = m_cubeTexture->target(),
                                    .texture = m_cubeTexture->handle(),
                                } }
        };
        m_descriptorSet = MAKE_SHARED(m_descriptorSet, m_render->device());
        m_descriptorSet->createDescriptorSetLayout(g_textureShaderResource);
        m_descriptorSet->createDescriptorSets(bufferInfos, imageInfos);
        m_pipeline->setDescriptorSet(m_descriptorSet);
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
        m_render->setPipeline(m_pipeline);
        for (unsigned int i = 0; i < g_cubePositions.size(); i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_cubePositions[i]);
            g_mvpMatrixUbo.model = glm::rotate(g_mvpMatrixUbo.model, m_duringTime, glm::vec3(0.5f, 1.0f, 0.0f));
            float angle = 20.0f * (float)i;
            g_mvpMatrixUbo.model = glm::rotate(g_mvpMatrixUbo.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            m_uniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(g_cubeVertex.size()));
        }
    }

private:
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_cubeTexture;
    std::shared_ptr<TextureGL> m_planeTexture;
    std::shared_ptr<BufferGL> m_planeVertexBuffer;
    std::shared_ptr<BufferGL> m_cubeVertexBuffer;
    std::shared_ptr<BufferGL> m_uniformBuffer;
    std::shared_ptr<DescriptorSetGl> m_descriptorSet;
};

void testDepthTestGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 800, 600, "OpenGL Example Depth Test" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestDepthTestGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}