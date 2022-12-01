//
// Created by cwb on 2022/9/22.
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

class TestTextureGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestTextureGl() override = default;
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
        m_uniformBuffer->create(sizeof(VertMVPMatrixUBO), &g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_render->device());
        m_vertexBuffer->create(g_quadVertex.size() * sizeof(g_quadVertex[0]), (void*)g_quadVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_indexBuffer = MAKE_SHARED(m_indexBuffer, m_render->device());
        m_indexBuffer->create(g_quadIndices.size() * sizeof(g_quadIndices[0]), (void*)g_quadIndices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::IndexBuffer);
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_render->device());
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void buildDescriptorsSets()
    {
        std::map<uint32_t, DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, DescriptorBufferInfo{
                                         .bufferType = m_uniformBuffer->bufferType(),
                                         .buffer = m_uniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        std::map<uint32_t, DescriptorImageInfo> imageInfos{
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

    void render() override
    {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        m_render->setPipeline(m_pipeline);
        glDrawElements(GL_TRIANGLES, (int32_t)g_quadIndices.size(), GL_UNSIGNED_SHORT, nullptr);
    }

private:
    std::shared_ptr<PipelineGL> m_pipeline;
    GLFWRendererGL* m_render{ nullptr };
    std::shared_ptr<TextureGL> m_texture;
    std::shared_ptr<BufferGL> m_vertexBuffer;
    std::shared_ptr<BufferGL> m_indexBuffer;
    std::shared_ptr<BufferGL> m_uniformBuffer;
    std::shared_ptr<DescriptorSetGl> m_descriptorSet;
};

void testTextureGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 640, 640, "OpenGL Example texture" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestTextureGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}