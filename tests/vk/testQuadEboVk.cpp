//
// Created by cwb on 2022/9/22.
//
#include "../mesh/globalMeshs.h"
#include "bufferVk.h"
#include "commonHandle.h"
#include "deviceVk.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "pipelineVk.h"
#include "utils.h"

using namespace backend;

namespace
{
class TestQuadVK : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestQuadVK() override = default;
    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVk*>(m_renderer->device());
        m_render = dynamic_cast<GLFWRendererVk*>(m_renderer);
        buildPipeline();
        buildBuffers();
    }

    void buildBuffers()
    {
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_deviceVk);
        m_vertexBuffer->create(g_quadVertex.size() * sizeof(g_quadVertex[0]), (void*)g_quadVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_indexBuffer = MAKE_SHARED(m_indexBuffer, m_deviceVk);
        m_indexBuffer->create(g_quadIndices.size() * sizeof(g_quadIndices[0]), (void*)g_quadIndices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::IndexBuffer);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVk);
        m_pipeline->setProgram(vertSource, fragShader);
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_pipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_pipeline->setTopology(backend::Topology::Triangles);
        m_pipeline->setPipelineLayout(m_pipelineLayout);
        m_pipeline->setViewport();
        m_pipeline->setRasterization();
        m_pipeline->setMultisample();
        m_pipeline->setColorBlendAttachment();
        m_pipeline->setRenderPass();
        m_pipeline->build();
        m_render->setPipeline(m_pipeline);
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        auto beginInfo = vk::CommandBufferBeginInfo{};
        auto renderPassInfo = m_deviceVk->getSingleRenderPassBeginInfo();
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            renderPassInfo.setFramebuffer(framebuffer[i]);
            commandBuffers[i].begin(beginInfo);
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->handle());
            commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
            commandBuffers[i].bindIndexBuffer(m_indexBuffer->buffer(), 0, vk::IndexType::eUint16);
            commandBuffers[i].drawIndexed(static_cast<std::uint32_t>(g_quadIndices.size()), 1, 0, 0, 0);
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVk* m_render{ nullptr };
    DeviceVk* m_deviceVk{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    std::shared_ptr<BufferVk> m_vertexBuffer;
    std::shared_ptr<BufferVk> m_indexBuffer;
    vk::PipelineLayout m_pipelineLayout;
};
} // namespace

void testQuadEboVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 480, "Vulkan Quad Use EBO" };
    DeviceVk handle(info);
    handle.init();
    GLFWRendererVk renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestQuadVK>(&renderer);
    engine.setEffect(effect);
    engine.run();
}
