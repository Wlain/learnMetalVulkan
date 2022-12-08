//
// Created by william on 2022/9/4.
//

#include "commonHandle.h"
#include "deviceVk.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "pipelineVk.h"

using namespace backend;

namespace
{
class TestWindowVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestWindowVk() override = default;
    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVk*>(m_renderer->device());
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        auto beginInfo = vk::CommandBufferBeginInfo{};
        static float red = 1.0f;
        red = red > 1.0 ? 0.0f : red + 0.01f;
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            commandBuffers[i].begin(beginInfo);
            std::array clearValues = {
                vk::ClearValue{ .color = { .float32 = std::array<float, 4>{ red, 0.0f, 0.0f, 1.0f } } },
                vk::ClearValue{ .depthStencil = { 1.0f, 0 } }
            };
            auto renderPassInfo = vk::RenderPassBeginInfo{
                .renderPass = m_deviceVk->renderPass(),
                .framebuffer = framebuffer[i],
                .renderArea = {
                    .offset = { 0, 0 },
                    .extent = m_deviceVk->swapchainExtent() },
                .clearValueCount = 2,
                .pClearValues = clearValues.data()
            };
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    DeviceVk* m_deviceVk{ nullptr };
};
} // namespace

void testWindowVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 480, "Vulkan Example Window" };
    DeviceVk handle(info);
    handle.init();
    GLFWRendererVk renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestWindowVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}