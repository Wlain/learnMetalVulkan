//
// Created by william on 2022/10/20.
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
class TestTextureMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestTextureMtl() override
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
        g_mvpMatrix.view = glm::translate(g_mvpMatrix.view, glm::vec3(0.0f, 0.0f, -3.0f));
    }

    void buildTexture()
    {
        m_texture = MAKE_SHARED(m_texture, m_device);
        m_texture->createWithFileName("/Users/william/git/learning/learnMetalVulkan/tests/textures/test.jpg", true);
    }

    void buildPipeline()
    {
        std::string vertSource = "#version 310 es\n"
                                 "precision highp float;\n"
                                 "\n"
                                 "layout (location = 0) in vec4 aPos;\n"
                                 "layout (location = 1) in vec4 aTexCoord;\n"
                                 "layout(location = 0) out vec2 vTexCoord;\n"
                                 "\n"
                                 "layout(binding = 2) uniform UniformBufferObject\n"
                                 "{\n"
                                 "    mat4 model;\n"
                                 "    mat4 view;\n"
                                 "    mat4 proj;\n"
                                 "} ubo;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "    vTexCoord = aTexCoord.xy;\n"
                                 "    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos.xyz, 1.0);\n"
                                 "}";
        std::string fragShader = "#version 310 es\n"
                                 "precision highp float;\n"
                                 "layout(location = 0) in vec2 vTexCoord;\n"
                                 "layout(location = 0) out vec4 fragColor;\n"
                                 "layout(binding = 1) uniform sampler2D inputTexture;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "    fragColor = texture(inputTexture, vTexCoord);\n"
                                 "}";
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

    void buildDepthStencilStates()
    {
        MTL::DepthStencilDescriptor* pDsDesc = MTL::DepthStencilDescriptor::alloc()->init();
        pDsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
        pDsDesc->setDepthWriteEnabled(true);
        m_depthStencilState = m_gpu->newDepthStencilState(pDsDesc);
        pDsDesc->release();
    }

    void resize(int width, int height) override
    {
        g_mvpMatrix.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }

    void update(float deltaTime) override
    {
        m_duringTime += deltaTime;
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
        g_mvpMatrix.model = glm::mat4(1.0f);
        g_mvpMatrix.model = glm::rotate(g_mvpMatrix.model, m_duringTime, glm::vec3(0.5f, 1.0f, 0.0f));
        auto* buffer = m_queue->commandBuffer();
        auto* encoder = buffer->renderCommandEncoder(pass);
        encoder->setRenderPipelineState(m_pipeline->pipelineState());
        encoder->setDepthStencilState(m_depthStencilState);

        encoder->setVertexBuffer(m_vertexBuffer->buffer(), 0, 0);
        encoder->setVertexBytes(&g_mvpMatrix, sizeof(g_mvpMatrix), 2); // ubo：小内存，大内存用buffer
        encoder->setFragmentTexture(m_texture->handle(), 1);
        encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVertex.size())));
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
    float m_duringTime{};
};

void testCubeMtl()
{
    Device::Info info{ Device::RenderType::Metal, 640, 640, "Metal Example Cube" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TestTextureMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}
