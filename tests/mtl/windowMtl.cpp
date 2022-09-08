//
// Created by william on 2022/9/4.
//

#include "commonHandle.h"
#include "engine.h"
#include "glfwRendererMtl.h"

class WindowMtl : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void initialize() override
    {
        auto* renderMtl = dynamic_cast<GLFWRendererMtl*>(m_renderer);
        m_swapChain = renderMtl->swapChain();
        m_queue = renderMtl->queue();
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
};

void windowMtl()
{
    GLFWRendererMtl rendererMtl;
    Engine engine(rendererMtl, "Metal Example window");
    auto effect = std::make_shared<WindowMtl>(&rendererMtl);
    engine.setEffect(effect);
    engine.run();
}