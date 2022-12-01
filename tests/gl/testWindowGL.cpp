//
// Created by william on 2022/9/4.
//

#include "deviceGl.h"
#include "effectBase.h"
#include "engine.h"
#include "glCommonDefine.h"
#include "glfwRendererGL.h"
using namespace backend;
class TestWindowGl : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void render() override
    {
        static float red = 1.0f;
        red = red > 1.0 ? 0.0f : red + 0.01f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};

void testWindowGl()
{
    Device::Info info{ DeviceGl::RenderType::OpenGL, 640, 480, "OpenGL Example window" };
    DeviceGl handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl);
    auto effect = std::make_shared<TestWindowGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}