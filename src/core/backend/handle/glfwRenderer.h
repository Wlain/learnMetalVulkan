//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_GLFWRENDERER_H
#define LEARNMETALVULKAN_GLFWRENDERER_H

#include "device.h"

#include <functional>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
namespace backend
{
class Pipeline;
// glfw抽象
class GLFWRenderer
{
public:
    explicit GLFWRenderer(Device* handle);
    virtual ~GLFWRenderer();
    GLFWRenderer(const GLFWRenderer&) = delete;

public:
    void init();
    void startEventLoop();
    GLFWwindow* glfwWindow();
    virtual void swapBuffers() = 0;
    virtual void setPipeline(const std::shared_ptr<Pipeline>& shader) = 0;
    [[nodiscard]] Device* device() const;

private:
    void frame(float deltaTimeSec);

public:
    // 方法回调
    std::function<void(int width, int height)> m_frameResize;
    std::function<void(double xPos, double yPos)> m_cursorPosEvent;
    std::function<void(int button, int action, int mods)> m_mouseButtonEvent;
    std::function<void(double xoffset, double yoffset)> m_scrollEvent;
    std::function<void(int key, int scancode, int action, int mods)> m_keyEvent;
    std::function<void(int count, const char** paths)> m_dropEvent;
    std::function<void(float deltaTimeSec)> m_frameUpdate;
    std::function<void()> m_frameRender;

protected:
    GLFWwindow* m_window{ nullptr };
    Device* m_device{ nullptr };

private:
    float m_timePerFrame{ 1.0f / 60.0f };
    bool m_running{ false };
};
} // namespace backend

#endif // LEARNMETALVULKAN_GLFWRENDERER_H
