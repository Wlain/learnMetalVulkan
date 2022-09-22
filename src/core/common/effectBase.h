//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_EFFECT_BASE_H
#define LEARNMETALVULKAN_EFFECT_BASE_H

#include "glfwRenderer.h"
namespace backend
{
class EffectBase
{
public:
    explicit EffectBase(GLFWRenderer* renderer);
    virtual ~EffectBase();
    virtual void initialize();
    virtual void resize(int width, int height);
    virtual void update(float deltaTime);
    virtual void render();
    virtual void finalize();
    virtual void setTitle();
    virtual void cursorPosEvent(double xPos, double yPos);
    // button: left or right, action:pressed or released
    virtual void mouseButtonEvent(int button, int action, int mods);
    virtual void dropEvent(int count, const char** paths);

protected:
    GLFWRenderer* m_renderer{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_EFFECT_BASE_H
