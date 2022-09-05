//
// Created by cwb on 2022/9/5.
//

#include "commonHandle.h"

CommonHandle* CommonHandle::create()
{
    return new CommonHandle();
}

CommonHandle::CommonHandle() = default;

CommonHandle::~CommonHandle() = default;

void CommonHandle::initialize()
{
    m_effects->initialize();
}

void CommonHandle::resize(int width, int height)
{
    m_effects->resize(width, height);
}

void CommonHandle::update(float deltaTime)
{
    m_effects->update(deltaTime);
}

void CommonHandle::render()
{
    m_effects->render();
}

void CommonHandle::finalize()
{
    m_effects->finalize();
}

void CommonHandle::cursorPosEvent(double xPos, double yPos)
{
    m_effects->cursorPosEvent(xPos, yPos);
}

void CommonHandle::mouseButtonEvent(int button, int action, int mods)
{
    m_effects->mouseButtonEvent(button, action, mods);
}

void CommonHandle::dropEvent(int count, const char** paths)
{
    m_effects->dropEvent(count, paths);
}

void CommonHandle::setEffect(const std::shared_ptr<EffectBase>& effect)
{
    m_effects = effect;
}