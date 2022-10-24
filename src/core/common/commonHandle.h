//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_COMMONHANDLE_H
#define LEARNMETALVULKAN_COMMONHANDLE_H
#include "effectBase.h"

#include <memory>
namespace backend
{
class CommonHandle
{
public:
    static CommonHandle* create();
    CommonHandle();
    virtual ~CommonHandle();
    virtual void initialize();
    virtual void resize(int width, int height);
    virtual void update(float deltaTime);
    virtual void render();
    virtual void finalize();
    virtual void cursorPosEvent(double xPos, double yPos);
    // button: left or right, action:pressed or released
    virtual void mouseButtonEvent(int button, int action, int mods);
    virtual void scrollEvent(double xoffset, double yoffset);
    virtual void dropEvent(int count, const char** paths);
    void setEffect(const std::shared_ptr<EffectBase>& effect);

protected:
    std::shared_ptr<EffectBase> m_effects;
};
} // namespace backend

#endif // LEARNMETALVULKAN_COMMONHANDLE_H
