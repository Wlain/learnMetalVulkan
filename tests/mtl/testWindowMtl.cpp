//
// Created by william on 2022/9/4.
//

#include "commonHandle.h"
#include "deviceMtl.h"
#include "engine.h"
#include "glfwRendererMtl.h"
using namespace backend;
class TestWindowMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestWindowMtl() override = default;
    void initialize() override
    {
        auto* renderMtl = dynamic_cast<GLFWRendererMtl*>(m_renderer);
        m_device = dynamic_cast<DeviceMtl*>(m_renderer->device());
        m_swapChain = m_device->swapChain();
        m_queue = m_device->queue();
    }

    void render() override
    {
        auto* surface = m_swapChain->nextDrawable();
        m_color.red = (m_color.red > 1.0) ? 0 : m_color.red + 0.01;
        MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::renderPassDescriptor();
        pass->colorAttachments()->object(0)->setClearColor(m_color);
        pass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        pass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
        pass->colorAttachments()->object(0)->setTexture(surface->texture());
        auto* buffer = m_queue->commandBuffer();
        auto* encoder = buffer->renderCommandEncoder(pass);
        encoder->endEncoding();
        buffer->presentDrawable(surface);
        buffer->commit();
    }

private:
    MTL::ClearColor m_color{ 0, 0, 0, 1 };
    CA::MetalLayer* m_swapChain{ nullptr };
    MTL::CommandQueue* m_queue{ nullptr };
    DeviceMtl* m_device{ nullptr };
};

void testWindowMtl()
{
    Device::Info info{ Device::RenderType::Metal, 640, 480, "Metal Example window" };
    DeviceMtl device(info);
    device.init();
    GLFWRendererMtl rendererMtl(&device);
    Engine engine(rendererMtl);
    auto effect = std::make_shared<TestWindowMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}