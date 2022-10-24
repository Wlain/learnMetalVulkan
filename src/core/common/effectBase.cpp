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
    m_width = width;
    m_height = height;
}

void EffectBase::update(float deltaTime)
{
    m_deltaTime = deltaTime;
    m_duringTime += deltaTime;
    if (glfwGetKey(m_renderer->glfwWindow(), GLFW_KEY_W) == GLFW_PRESS)
        m_camera.processKeyboard(Camera::MovementType::Forward, deltaTime);
    if (glfwGetKey(m_renderer->glfwWindow(), GLFW_KEY_S) == GLFW_PRESS)
        m_camera.processKeyboard(Camera::MovementType::BackWard, deltaTime);
    if (glfwGetKey(m_renderer->glfwWindow(), GLFW_KEY_A) == GLFW_PRESS)
        m_camera.processKeyboard(Camera::MovementType::Left, deltaTime);
    if (glfwGetKey(m_renderer->glfwWindow(), GLFW_KEY_D) == GLFW_PRESS)
        m_camera.processKeyboard(Camera::MovementType::Right, deltaTime);
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

void EffectBase::cursorPosEvent(double posx, double posy)
{
    auto xpos = static_cast<float>(posx);
    auto ypos = static_cast<float>(posy);
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

void EffectBase::mouseButtonEvent(int button, int action, int mods)
{
}

void EffectBase::dropEvent(int count, const char** paths)
{
}

void EffectBase::scrollEvent(double xoffset, double yoffset)
{
    m_camera.processMouseScroll(static_cast<float>(yoffset));
}
} // namespace backend