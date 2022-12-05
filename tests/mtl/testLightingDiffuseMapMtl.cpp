//
// Created by william on 2022/12/2.
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
class TestDiffuseMapCubeMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestDiffuseMapCubeMtl() override
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
        buildTexture();
        buildDepthStencilStates();
        setupDepthStencilTexture();
    }

    void buildPipeline()
    {
        // light sphere
        std::string vertSource = getFileContents("shaders/lightColorCube.vert");
        std::string fragShader = getFileContents("shaders/lightColorCube.frag");
        m_lightSpherePipeline = MAKE_SHARED(m_lightSpherePipeline, m_device);
        m_lightSpherePipeline->setProgram(vertSource, fragShader);
        m_lightSpherePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_lightSpherePipeline->build();

        // diffuseMapCube
        vertSource = getFileContents("shaders/lightingDiffuseMap.vert");
        fragShader = getFileContents("shaders/lightingDiffuseMap.frag");
        m_diffuseMapCubePipeline = MAKE_SHARED(m_diffuseMapCubePipeline, m_device);
        m_diffuseMapCubePipeline->setProgram(vertSource, fragShader);
        m_diffuseMapCubePipeline->setAttributeDescription(getThreeElemsAttributesDescriptions());
        m_diffuseMapCubePipeline->build();
    }

    void buildTexture()
    {
        m_diffuseMapTexture = MAKE_SHARED(m_diffuseMapTexture, m_device);
        m_diffuseMapTexture->createWithFileName("textures/container.jpg", true);
        m_specularMapTexture = MAKE_SHARED(m_specularMapTexture, m_device);
        m_specularMapTexture->createWithFileName("textures/container2_specular.png", true);
        m_emissionMapTexture = MAKE_SHARED(m_emissionMapTexture, m_device);
        m_emissionMapTexture->createWithFileName("textures/matrix.jpg", true);
    }

    void buildBuffers()
    {
        m_lightSphereVertexBuffer = MAKE_SHARED(m_lightSphereVertexBuffer, m_device);
        m_lightSphereVertexBuffer->create(sizeof(g_sphereMesh[0]) * g_sphereMesh.size(), (void*)g_sphereMesh.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);

        m_diffuseMapCubeVertexBuffer = MAKE_SHARED(m_diffuseMapCubeVertexBuffer, m_device);
        m_diffuseMapCubeVertexBuffer->create(sizeof(g_cubeVerticesWithNormalTexCoord[0]) * g_cubeVerticesWithNormalTexCoord.size(), (void*)g_cubeVerticesWithNormalTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
    }

    void setupDepthStencilTexture()
    {
        m_depthTexture = MAKE_SHARED(m_depthTexture, m_device);
        m_depthTexture->createDepthTexture((int)m_device->width(), (int)m_device->height(), Texture::DepthPrecision::F32);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
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
        // draw lighting sphere
        {
            encoder->setRenderPipelineState(m_lightSpherePipeline->pipelineState());
            encoder->setDepthStencilState(m_depthStencilState);
            encoder->setVertexBuffer(m_lightSphereVertexBuffer->buffer(), 0, 0);
            g_fragDiffuseMapUBO.light.position.x = 1.0f + sin(m_duringTime) * 2.0f;
            g_fragDiffuseMapUBO.light.position.y = sin(m_duringTime / 2.0f) * 1.0f;
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_fragDiffuseMapUBO.light.position));
            g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.05f));               // a smaller cube
            encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
            g_lightColorUbo.lightColor.x = static_cast<float>(std::abs(sin(m_duringTime * 2.0)));
            g_lightColorUbo.lightColor.y = static_cast<float>(std::abs(sin(m_duringTime * 0.7)));
            g_lightColorUbo.lightColor.z = static_cast<float>(std::abs(sin(m_duringTime * 1.3)));
            encoder->setFragmentBytes(&g_lightColorUbo, sizeof(g_lightColorUbo), g_lightSphereUboBinding); // ubo：小内存，大内存用buffer
            encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_sphereMesh.size())));
        }
        // draw diffuseMap cube
        {
            encoder->setRenderPipelineState(m_diffuseMapCubePipeline->pipelineState());
            encoder->setDepthStencilState(m_depthStencilState);
            encoder->setVertexBuffer(m_diffuseMapCubeVertexBuffer->buffer(), 0, 0);
            // calculate the model matrix for each object and pass it to shader before drawing
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            encoder->setVertexBytes(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), g_mvpMatrixUboBinding); // ubo：小内存，大内存用buffer
            g_fragDiffuseMapUBO.viewPos = glm::vec4(m_camera.position, 1.0f);
            g_fragDiffuseMapUBO.light.diffuse = g_lightColorUbo.lightColor * glm::vec4(0.5f);            // decrease the influence;
            g_fragDiffuseMapUBO.light.ambient = g_fragDiffuseMapUBO.light.diffuse * glm::vec4(0.2f); // decrease the influence
            g_fragDiffuseMapUBO.light.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
            encoder->setFragmentBytes(&g_fragDiffuseMapUBO, sizeof(g_fragDiffuseMapUBO), g_fragDiffuseMapUboBinding);
            encoder->setFragmentTexture(m_diffuseMapTexture->handle(), g_diffuseTextureBinding);
            encoder->setFragmentTexture(m_specularMapTexture->handle(), g_specularTextureBinding);
            encoder->setFragmentTexture(m_emissionMapTexture->handle(), g_emissionTextureBinding);
            encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(static_cast<uint32_t>(g_cubeVerticesWithNormalTexCoord.size())));
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
    std::shared_ptr<PipelineMtl> m_lightSpherePipeline;
    std::shared_ptr<PipelineMtl> m_diffuseMapCubePipeline;
    std::shared_ptr<TextureMTL> m_diffuseMapTexture;
    std::shared_ptr<TextureMTL> m_specularMapTexture;
    std::shared_ptr<TextureMTL> m_emissionMapTexture;
    std::shared_ptr<TextureMTL> m_depthTexture;
    std::shared_ptr<BufferMTL> m_lightSphereVertexBuffer;
    std::shared_ptr<BufferMTL> m_diffuseMapCubeVertexBuffer;
    MTL::DepthStencilState* m_depthStencilState{};
};
} // namespace

void testLightingDiffuseMapMtl()
{
    Device::Info info{ Device::RenderType::Metal, 800, 600, "Metal Example diffuseMap" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TestDiffuseMapCubeMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}