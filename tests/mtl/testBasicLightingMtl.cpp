//
// Created by william on 2022/11/2.
//

#include "../mesh/globalMeshs.h"
#include "bufferMtl.h"
#include "commonHandle.h"
#include "commonMacro.h"
#include "depthStencilStateMtl.h"
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
class TestBasicLightingMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestBasicLightingMtl() override = default;
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
    }

    void buildPipeline()
    {
        // lightCube
        std::string vertSource = getFileContents("shaders/lightCube.vert");
        std::string fragShader = getFileContents("shaders/lightCube.frag");
        m_lightCubePipeline = MAKE_SHARED(m_lightCubePipeline, m_device);
        m_lightCubePipeline->setProgram(vertSource, fragShader);
        m_lightCubePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_lightCubePipeline->build();
        // basic lighting
        vertSource = getFileContents("shaders/basicLighting.vert");
        fragShader = getFileContents("shaders/basicLighting.frag");
        m_basicLightingPipeline = MAKE_SHARED(m_basicLightingPipeline, m_device);
        m_basicLightingPipeline->setProgram(vertSource, fragShader);
        m_basicLightingPipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_basicLightingPipeline->build();
    }

    void buildBuffers()
    {
        m_vertexBufferLightingCube = MAKE_SHARED(m_vertexBufferLightingCube, m_device);
        m_vertexBufferLightingCube->create(sizeof(g_cubeVertices[0]) * g_cubeVertices.size(), (void*)g_cubeVertices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);

        m_vertexBufferBasicLighting = MAKE_SHARED(m_vertexBufferBasicLighting, m_device);
        m_vertexBufferBasicLighting->create(sizeof(g_cubeVerticesWithNormal[0]) * g_cubeVerticesWithNormal.size(), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
    }

    void setupDepthStencilTexture()
    {
        m_depthTexture = MAKE_SHARED(m_depthTexture, m_device);
        m_depthTexture->createDepthTexture(m_device->width(), m_device->height(), Texture::DepthPrecision::F32);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void buildDepthStencilStates()
    {
        m_depthStencilState = MAKE_SHARED(m_depthStencilState, m_device);
        m_depthStencilState->setDepthCompareOp(CompareOp::Less);
        m_depthStencilState->setDepthTestEnable(true);
        m_depthStencilState->setDepthWriteEnable(true);
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
        // draw lighting cube
        {
            encoder->setRenderPipelineState(m_lightCubePipeline->pipelineState());
            encoder->setDepthStencilState(m_depthStencilState->handle());
            encoder->setVertexBuffer(m_vertexBufferLightingCube->buffer(), 0, 0);
            // calculate the model matrix for each object and pass it to shader before drawing
            g_basicLightingColorUbo.lightPos.x = 1.0f + sin(m_duringTime) * 2.0f;
            g_basicLightingColorUbo.lightPos.y = sin(m_duringTime / 2.0f) * 1.0f;
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_basicLightingColorUbo.lightPos));
            g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f));                // a smaller cube
            encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
            encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVerticesWithNormal.size())));
        }
        // draw basic lighting cube
        {
            encoder->setRenderPipelineState(m_basicLightingPipeline->pipelineState());
            encoder->setDepthStencilState(m_depthStencilState->handle());
            encoder->setVertexBuffer(m_vertexBufferBasicLighting->buffer(), 0, 0);
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
            g_basicLightingColorUbo.viewPos = glm::vec4(m_camera.position, 1.0f);
            encoder->setFragmentBytes(&g_basicLightingColorUbo, sizeof(g_basicLightingColorUbo), g_basicLightingColorUboBinding);
            encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVerticesWithNormal.size())));
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
    std::shared_ptr<PipelineMtl> m_lightCubePipeline;
    std::shared_ptr<PipelineMtl> m_basicLightingPipeline;
    std::shared_ptr<TextureMTL> m_depthTexture;
    std::shared_ptr<BufferMTL> m_vertexBufferLightingCube;
    std::shared_ptr<BufferMTL> m_vertexBufferBasicLighting;
    std::shared_ptr<DepthStencilStateMTL> m_depthStencilState;
};
} // namespace

void testBasicLightingMtl()
{
    Device::Info info{ Device::RenderType::Metal, 800, 600, "Metal Example Basic Lighting Diffuse" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TestBasicLightingMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}