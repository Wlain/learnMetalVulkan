//
// Created by cwb on 2022/9/22.
//

#include "textureMtl.h"

#include "../mesh/globalMeshs.h"
#include "commonHandle.h"
#include "commonMacro.h"
#include "deviceMtl.h"
#include "engine.h"
#include "glfwRendererMtl.h"
#include "pipelineMtl.h"
#include "utils/utils.h"

#include <glm/glm.hpp>
#include <vector>

using namespace backend;
class TextureMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TextureMtl() override
    {
        m_vertexBuffer->release();
    }
    void initialize() override
    {
        m_device = dynamic_cast<DeviceMtl*>(m_renderer->device());
        m_swapChain = m_device->swapChain();
        m_queue = m_device->queue();
        m_gpu = m_device->gpu();
        buildPipeline();
        buildBuffers();
        buildTexture();
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_device);
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/checkTexCoord.vert");
        std::string fragShader = getFileContents("shaders/checkTexCoord.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_device);
        m_pipeline->setProgram(vertSource, fragShader);
    }

    void buildBuffers()
    {
        auto vertexDataSize = g_quadVertex.size() * sizeof(g_quadVertex[0]);
        // static:使用MTL::ResourceCPUCacheModeWriteCombined会获得性能提升
        // other:MTLResourceStorageModeShared
        auto usage = MTL::ResourceCPUCacheModeWriteCombined;
        m_vertexBuffer = m_gpu->newBuffer(vertexDataSize, usage);
        std::memcpy(m_vertexBuffer->contents(), g_quadVertex.data(), vertexDataSize);
        // 通知Metal有关已修改的特定数据范围; 这允许Metal仅更新视频内存副本中的特定范围
        m_vertexBuffer->didModifyRange({ 0, m_vertexBuffer->length() });
    }

    void render() override
    {
        auto* surface = m_swapChain->nextDrawable();
        m_color.red = 1.0f; //(m_color.red > 1.0) ? 0 : m_color.red + 0.01;
        MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::renderPassDescriptor();
        pass->colorAttachments()->object(0)->setClearColor(m_color);
        pass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        pass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
        pass->colorAttachments()->object(0)->setTexture(surface->texture());
        auto* buffer = m_queue->commandBuffer();
        auto* encoder = buffer->renderCommandEncoder(pass);
        encoder->setRenderPipelineState(m_pipeline->pipelineState());
        encoder->setVertexBuffer(m_vertexBuffer, 0, 0);
        encoder->setFragmentTexture(m_texture->handle(), 0);
        encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
        encoder->endEncoding();
        buffer->presentDrawable(surface);
        buffer->commit();
    }

private:
    MTL::ClearColor m_color{ 0, 0, 0, 1 };
    CA::MetalLayer* m_swapChain{ nullptr };
    MTL::CommandQueue* m_queue{ nullptr };
    DeviceMtl* m_device{ nullptr };
    MTL::Device* m_gpu{ nullptr };
    MTL::Buffer* m_vertexBuffer{ nullptr };
    std::shared_ptr<PipelineMtl> m_pipeline;
    std::shared_ptr<TextureMTL> m_texture;
};

void textureMtl()
{
    Device::Info info{ Device::RenderType::Metal, 480, 480, "Metal Example texture" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TextureMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}