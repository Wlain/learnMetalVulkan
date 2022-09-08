//
// Created by cwb on 2022/9/6.
//
#include "commonHandle.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "utils.h"

class Window : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragmentShader = getFileContents("shaders/triangle.frag");
        m_render->initPipeline(vertSource, fragmentShader);
        m_render->initCommandBuffer();
    }

    void render() override
    {
        m_render->commit();
    }

private:
    GLFWRendererVK* m_render{ nullptr };
};

void triangleVk()
{
    GLFWRendererVK renderer;
    Engine engine(renderer, "Vulkan Example Test");
    auto effect = std::make_shared<Window>(&renderer);
    engine.setEffect(effect);
    engine.run();
}