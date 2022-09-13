//
// Created by cwb on 2022/9/6.
//
#include "commonHandle.h"
#include "deviceVk.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "pipelineVk.h"
#include "utils.h"

using namespace backend;
namespace
{
class Triangle : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void initialize() override
    {
        m_device = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_device);
        m_pipeline->setProgram(vertSource, fragShader);
        m_pipeline->initVertexBuffer();
        m_pipeline->setAssembly();
        m_pipeline->setPipelineLayout();
        m_pipeline->setViewport();
        m_pipeline->setRasterization();
        m_pipeline->setMultisample();
        m_pipeline->setDepthStencil();
        m_pipeline->setColorBlendAttachment();
        m_pipeline->setRenderPass();
        m_pipeline->build();
        m_render->setPipeline(m_pipeline);
        m_render->initSemaphore();
        m_render->initCommandBuffer();
    }

    void render() override
    {
        m_render->commit();
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_device{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
};
} // namespace

void triangleVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 480, "Vulkan Triangle" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<Triangle>(&renderer);
    engine.setEffect(effect);
    engine.run();
}
