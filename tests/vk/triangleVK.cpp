//
// Created by cwb on 2022/9/6.
//
#include "commonHandle.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "deviceVk.h"
#include "pipeline.h"
#include "utils.h"
using namespace backend;
class Window : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragmentShader = getFileContents("shaders/triangle.frag");
        //        m_render->setPipeline(m_pipeline);
        m_render->initCommandBuffer();
    }

    void render() override
    {
        m_render->commit();
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    std::shared_ptr<Pipeline> m_pipeline;
};

void triangleVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 480 };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer, "Vulkan Example Test");
    auto effect = std::make_shared<Window>(&renderer);
    engine.setEffect(effect);
    engine.run();
}