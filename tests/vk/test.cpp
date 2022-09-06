//
// Created by cwb on 2022/9/6.
//
#include "commonHandle.h"
#include "engine.h"
#include "glfwRendererVK.h"

class Test : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void initialize() override
    {
    }

    void render() override
    {
    }

private:
};

void testVK()
{
    GLFWRendererVK renderer;
    Engine engine(renderer, "Vulkan Example Test");
    auto effect = std::make_shared<Test>(&renderer);
    engine.setEffect(effect);
    engine.run();
}