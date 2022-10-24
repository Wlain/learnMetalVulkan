//
// Created by cwb on 2022/9/5.
//

#include "effectBase.h"

#include <GLFW/glfw3.h>

namespace backend
{
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
    m_deltaTime = deltaTime;
    m_duringTime += deltaTime;
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
    m_camera.processMouseScroll(static_cast<float>(yPos));
}

void EffectBase::mouseButtonEvent(int button, int action, int mods)
{
}

void EffectBase::dropEvent(int count, const char** paths)
{
}

void EffectBase::keyEvent(int key, int scancode, int action, int mods)
{
    Camera::MovementType moveMode{};
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        moveMode = Camera::MovementType::Forward;
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        moveMode = Camera::MovementType::BackWard;
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        moveMode = Camera::MovementType::Left;
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
        moveMode = Camera::MovementType::Right;
    m_camera.processKeyboard(moveMode, m_deltaTime);
}

void EffectBase::scrollEvent(double xoffset, double yoffset)
{
    auto xpos = static_cast<float>(xoffset);
    auto ypos = static_cast<float>(yoffset);
    if (!m_isFirstMouse)
    {
        m_isFirstMouse = true;
        m_lastPosX = xpos;
        m_lastPosY = ypos;
    }
    // // reversed since y-coordinates go from bottom to top
    glm::vec2 offset{ xpos - m_lastPosX, m_lastPosY - ypos };
    m_lastPosX = xpos;
    m_lastPosY = ypos;
    m_camera.processMouseMovement(offset.x, offset.y);
}
} // namespace backend