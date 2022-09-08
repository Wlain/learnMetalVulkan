//
// Created by cwb on 2022/9/8.
//

#include "effectBase.h"
#include "engine.h"
#include "glfwRendererGL.h"
#include "deviceGL.h"
using namespace backend;

class TriangleGl : public EffectBase
{
public:
    using EffectBase::EffectBase;
    void initialize() override
    {
    }
    void render() override
    {
        static float red = 1.0f;
        red = red > 1.0 ? 0.0f : red + 0.01f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

private:
};

void triangleGl()
{
    Device::Info info{ Device::RenderType::OpenGL, 640, 480 };
    HandleGL handle(info);
    handle.init();
    GLFWRendererGL rendererGl(&handle);
    Engine engine(rendererGl, "OpenGL Example window");
    auto effect = std::make_shared<TriangleGl>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}
