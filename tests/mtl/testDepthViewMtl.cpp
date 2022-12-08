//
// Created by william on 2022/12/7.
//
#include "../mesh/globalMeshs.h"
#include "bufferMtl.h"
#include "commonMacro.h"
#include "depthStencilStateMtl.h"
#include "deviceMtl.h"
#include "effectBase.h"
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
class TestDepthViewMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestDepthViewMtl() override
    {
        m_samplerState->release();
    }
    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererMtl*>(m_renderer);
        m_device = dynamic_cast<DeviceMtl*>(m_renderer->device());
        m_swapChain = m_device->swapChain();
        m_queue = m_device->queue();
        buildPipeline();
        buildBuffers();
        buildDepthStencilStates();
        setupDepthStencilTexture();
        buildTexture();
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/depth.vert");
        std::string fragShader = getFileContents("shaders/depth.frag");
        m_planePipeline = MAKE_SHARED(m_planePipeline, m_render->device());
        m_planePipeline->setProgram(vertSource, fragShader);
        m_planePipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_planePipeline->build();

        m_cubePipeline = MAKE_SHARED(m_cubePipeline, m_render->device());
        m_cubePipeline->setProgram(vertSource, fragShader);
        m_cubePipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_cubePipeline->build();
    }

    void buildTexture()
    {
        m_cubeTexture = MAKE_SHARED(m_cubeTexture, m_render->device());
        m_cubeTexture->createWithFileName("textures/marble.jpg", true);
        m_planeTexture = MAKE_SHARED(m_cubeTexture, m_render->device());
        m_planeTexture->createWithFileName("textures/metal.png", true);
        MTL::SamplerDescriptor* samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
        samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilter::SamplerMinMagFilterLinear);
        samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilter::SamplerMinMagFilterLinear);
        samplerDescriptor->setMipFilter(MTL::SamplerMipFilter::SamplerMipFilterLinear);
        samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeRepeat);
        samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeRepeat);
        samplerDescriptor->setRAddressMode(MTL::SamplerAddressModeRepeat);
        m_samplerState = m_device->gpu()->newSamplerState(samplerDescriptor);
        samplerDescriptor->release();
    }

    void buildBuffers()
    {
        m_cubeVertexBuffer = MAKE_SHARED(m_cubeVertexBuffer, m_render->device());
        m_cubeVertexBuffer->create(g_cubeVerticesPosTexCoord.size() * sizeof(g_cubeVerticesPosTexCoord[0]), (void*)g_cubeVerticesPosTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);

        m_planeVertexBuffer = MAKE_SHARED(m_planeVertexBuffer, m_render->device());
        m_planeVertexBuffer->create(g_planeVerticesPosTexCoord.size() * sizeof(g_planeVerticesPosTexCoord[0]), (void*)g_planeVerticesPosTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
    }

    void setupDepthStencilTexture()
    {
        m_depthTexture = MAKE_SHARED(m_depthTexture, m_device);
        m_depthTexture->createDepthTexture(m_device->width(), m_device->height(), Texture::DepthPrecision::F32);
    }

    void buildDepthStencilStates()
    {
        m_depthStencilState = MAKE_SHARED(m_depthStencilState, m_render->device());
        m_depthStencilState->setDepthCompareOp(CompareOp::Less);
        m_depthStencilState->setDepthTestEnable(true);
        m_depthStencilState->setDepthWriteEnable(true);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void render() override
    {
        auto* surface = m_swapChain->nextDrawable();
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
        // cubes
        encoder->setRenderPipelineState(m_cubePipeline->pipelineState());
        encoder->setDepthStencilState(m_depthStencilState->handle());
        encoder->setVertexBuffer(m_cubeVertexBuffer->buffer(), 0, 0);
        encoder->setFragmentTexture(m_cubeTexture->handle(), g_textureBinding);
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(-1.0f, 0.0f, -1.0f));
        encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
        encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVerticesPosTexCoord.size())));

        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(2.0f, 0.0f, 0.0f));
        encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
        encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVerticesPosTexCoord.size())));
        // floor
        encoder->setRenderPipelineState(m_planePipeline->pipelineState());
        encoder->setDepthStencilState(m_depthStencilState->handle());
        encoder->setVertexBuffer(m_planeVertexBuffer->buffer(), 0, 0);
        encoder->setFragmentTexture(m_planeTexture->handle(), g_textureBinding);
        encoder->setFragmentSamplerState(m_samplerState, g_textureBinding);
        g_mvpMatrixUbo.model = glm::mat4(1.0f);
        encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
        encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_planeVerticesPosTexCoord.size())));

        encoder->endEncoding();
        buffer->presentDrawable(surface);
        buffer->commit();
    }

private:
    GLFWRendererMtl* m_render{ nullptr };
    MTL::ClearColor m_color{ 0.1f, 0.1f, 0.1f, 1.0f };
    CA::MetalLayer* m_swapChain{ nullptr };
    MTL::CommandQueue* m_queue{ nullptr };
    DeviceMtl* m_device{ nullptr };
    std::shared_ptr<DepthStencilStateMTL> m_depthStencilState;
    std::shared_ptr<TextureMTL> m_depthTexture;
    MTL::SamplerState* m_samplerState;

    std::shared_ptr<PipelineMtl> m_planePipeline;
    std::shared_ptr<TextureMTL> m_planeTexture;
    std::shared_ptr<BufferMTL> m_planeVertexBuffer;

    std::shared_ptr<PipelineMtl> m_cubePipeline;
    std::shared_ptr<TextureMTL> m_cubeTexture;
    std::shared_ptr<BufferMTL> m_cubeVertexBuffer;
};
} // namespace

void testDepthViewMtl()
{
    Device::Info info{ Device::RenderType::Metal, 800, 600, "Metal Example Depth View" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TestDepthViewMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}