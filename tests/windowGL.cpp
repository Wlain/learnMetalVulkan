//
// Created by william on 2022/9/4.
//

#include "effectBase.h"
#include "engine.h"
#include "glfwRendererGL.h"

class WindowGL : public EffectBase
{
public:
    using EffectBase::EffectBase;
    void render() override
    {
        static float red = 0.0f;
        red = red > 1.0 ? 0.0f : red + 0.01f;
        glClearColor(red, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};

void windowGL()
{
    GLFWRendererGL rendererGl;
    Engine engine(rendererGl, "OpenGL Example window");
    auto effect = std::make_shared<WindowGL>(&rendererGl);
    engine.setEffect(effect);
    engine.run();
}