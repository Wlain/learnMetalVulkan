//
// Created by cwb on 2022/9/6.
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
class TestTriangleVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestTriangleVk() override = default;
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
        m_vertexBuffer->create(g_triangleVertex.size() * sizeof(g_triangleVertex[0]), (void*)g_triangleVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVk);
        m_pipeline->setProgram(vertSource, fragShader);
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0
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
            commandBuffers[i].draw(static_cast<std::uint32_t>(g_triangleVertex.size()), 1, 0, 0);
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVk* m_render{ nullptr };
    DeviceVk* m_deviceVk{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    std::shared_ptr<BufferVk> m_vertexBuffer;
    vk::PipelineLayout m_pipelineLayout;
};
} // namespace

void testTriangleVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 480, "Vulkan Triangle" };
    DeviceVk handle(info);
    handle.init();
    GLFWRendererVk renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestTriangleVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}
