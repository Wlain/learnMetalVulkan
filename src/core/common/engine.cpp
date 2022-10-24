//
// Created by cwb on 2022/9/5.
//

#include "engine.h"
namespace backend
{
Engine::Engine(GLFWRenderer& renderer) :
    m_renderer(renderer)
{
    m_renderer.init();
    m_handle.reset(CommonHandle::create());
};

void Engine::setEffect(const std::shared_ptr<EffectBase>& effect)
{
    m_handle->setEffect(effect);
}

void Engine::run()
{
    m_handle->initialize();
    m_handle->resize(m_renderer.device()->width(), m_renderer.device()->height());
    m_renderer.m_frameUpdate = [&](float deltaTime) {
        m_handle->update(deltaTime);
    };
    m_renderer.m_frameResize = [&](int width, int height) {
        m_handle->resize(width, height);
    };
    m_renderer.m_cursorPosEvent = [&](int xPos, int yPos) {
        m_handle->cursorPosEvent(xPos, yPos);
    };
    m_renderer.m_mouseButtonEvent = [&](int button, int action, int mods) {
        m_handle->mouseButtonEvent(button, action, mods);
    };
    m_renderer.m_dropEvent = [&](int count, const char** paths) {
        m_handle->dropEvent(count, paths);
    };
    m_renderer.m_keyEvent = [&](int key, int scancode, int action, int mods) {
        m_handle->keyEvent(key, scancode, action, mods);
    };
    m_renderer.m_scrollEvent = [&](double xoffset, double yoffset) {
        m_handle->scrollEvent(xoffset, yoffset);
    };
    m_renderer.m_frameRender = [&]() {
        m_handle->render();
    };
    m_renderer.startEventLoop();
}
} // namespace backend