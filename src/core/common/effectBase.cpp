//
// Created by cwb on 2022/9/5.
//

#include "effectBase.h"

EffectBase::EffectBase(GLFWRenderer* renderer) :
    m_renderer(renderer)
{
}

EffectBase::~EffectBase() = default;

void EffectBase::initialize()
{
}

void EffectBase::resize(int width, int height)
{
}

void EffectBase::update(float deltaTime)
{
}

void EffectBase::render()
{
}

void EffectBase::finalize()
{
}

void EffectBase::setTitle()
{
}

void EffectBase::cursorPosEvent(double xPos, double yPos)
{
}

void EffectBase::mouseButtonEvent(int button, int action, int mods)
{
}

void EffectBase::dropEvent(int count, const char** paths)
{
}
