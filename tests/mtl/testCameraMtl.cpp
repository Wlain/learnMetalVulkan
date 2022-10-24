//
// Created by william on 2022/10/24.
//


#include "../mesh/globalMeshs.h"
#include "bufferMtl.h"
#include "commonHandle.h"
#include "commonMacro.h"
#include "deviceMtl.h"
#include "engine.h"
#include "glfwRendererMtl.h"
#include "pipelineMtl.h"
#include "textureMtl.h"
#include "utils/utils.h"

#include <glm/glm.hpp>
#include <vector>

using namespace backend;
namespace
{
class TestCameraMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestCameraMtl() override
    {
        m_depthStencilState->release();
    }
    void initialize() override
    {
        m_device = dynamic_cast<DeviceMtl*>(m_renderer->device());
        m_swapChain = m_device->swapChain();
        m_queue = m_device->queue();
        m_gpu = m_device->gpu();
        buildPipeline();
        buildBuffers();
        buildDepthStencilStates();
        setupDepthStencilTexture();
        buildTexture();
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_device);
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/texture.vert");
        std::string fragShader = getFileContents("shaders/texture.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_device);
        m_pipeline->setProgram(vertSource, fragShader);
    }

    void buildBuffers()
    {
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_device);
        m_vertexBuffer->create(sizeof(g_cubeVertex[0]) * g_cubeVertex.size(), (void*)g_cubeVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
    }

    void setupDepthStencilTexture()
    {
        m_depthTexture = MAKE_SHARED(m_depthTexture, m_device);
        m_depthTexture->createDepthTexture(m_device->width(), m_device->height(), Texture::DepthPrecision::F32);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrix.view = m_camera.viewMatrix();
        g_mvpMatrix.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void buildDepthStencilStates()
    {
        MTL::DepthStencilDescriptor* pDsDesc = MTL::DepthStencilDescriptor::alloc()->init();
        pDsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
        pDsDesc->setDepthWriteEnabled(true);
        m_depthStencilState = m_gpu->newDepthStencilState(pDsDesc);
        pDsDesc->release();
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
        pass->depthAttachment()->setTexture(m_depthTexture->handle());
        pass->depthAttachment()->setClearDepth(1.0);
        pass->stencilAttachment()->setClearStencil(0);
        auto* buffer = m_queue->commandBuffer();
        auto* encoder = buffer->renderCommandEncoder(pass);
        encoder->setRenderPipelineState(m_pipeline->pipelineState());
        encoder->setDepthStencilState(m_depthStencilState);
        encoder->setVertexBuffer(m_vertexBuffer->buffer(), 0, 0);
        encoder->setFragmentTexture(m_texture->handle(), 1);
        for (unsigned int i = 0; i < g_cubePositions.size(); i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrix.model = glm::mat4(1.0f);
            g_mvpMatrix.model = glm::translate(g_mvpMatrix.model, g_cubePositions[i]);
            g_mvpMatrix.model = glm::rotate(g_mvpMatrix.model, m_duringTime, glm::vec3(0.5f, 1.0f, 0.0f));
            float angle = 20.0f * i;
            g_mvpMatrix.model = glm::rotate(g_mvpMatrix.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            encoder->setVertexBytes(&g_mvpMatrix, sizeof(g_mvpMatrix), 2); // ubo：小内存，大内存用buffer
            encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVertex.size())));
        }
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
    std::shared_ptr<PipelineMtl> m_pipeline;
    std::shared_ptr<TextureMTL> m_texture;
    std::shared_ptr<TextureMTL> m_depthTexture;
    std::shared_ptr<BufferMTL> m_vertexBuffer;
    MTL::DepthStencilState* m_depthStencilState;
};
} // namespace

void testCameraMtl()
{
    Device::Info info{ Device::RenderType::Metal, 640, 640, "Metal Example Cube Multiple" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TestCameraMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}