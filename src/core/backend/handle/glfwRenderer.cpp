//
// Created by cwb on 2022/9/5.
//

#include "glfwRenderer.h"

#include "commonMacro.h"

#include <GLFW/glfw3.h>
namespace backend
{
GLFWRenderer::GLFWRenderer(Device* device) :
    m_frameRender([]() {}),
    m_frameUpdate([](float) {}),
    m_device{ device }
{
    ASSERT(device);
    m_window = device->window();
}

GLFWRenderer::~GLFWRenderer()
{
    glfwTerminate();
}

void GLFWRenderer::init()
{
    if (m_window)
    {
        // glfw window creation
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            auto* app = reinterpret_cast<GLFWRenderer*>(glfwGetWindowUserPointer(window));
            app->m_frameResize(width, height);
        });
        // 设置光标回调
        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
            auto* app = reinterpret_cast<GLFWRenderer*>(glfwGetWindowUserPointer(window));
            app->m_cursorPosEvent(xPos, yPos);
        });
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
            auto* app = reinterpret_cast<GLFWRenderer*>(glfwGetWindowUserPointer(window));
            app->m_mouseButtonEvent(button, action, mods);
        });

        glfwSetDropCallback(m_window, [](GLFWwindow* window, int count, const char** paths) {
            auto* app = reinterpret_cast<GLFWRenderer*>(glfwGetWindowUserPointer(window));
            app->m_dropEvent(count, paths);
        });
        if (m_window == nullptr)
        {
            glfwTerminate();
        }
    }
}

void GLFWRenderer::startEventLoop()
{
    if (!m_window)
    {
        throw std::runtime_error("GLFWRenderer::init() not called");
    }
    m_running = true;
    typedef std::chrono::high_resolution_clock Clock;
    using FpSeconds = std::chrono::duration<float, std::chrono::seconds::period>;
    auto lastTick = Clock::now();
    float deltaTime = 0;
    while (m_running)
    {
        frame(deltaTime);
        auto tick = Clock::now();
        deltaTime = std::chrono::duration_cast<FpSeconds>(tick - lastTick).count();
        lastTick = tick;
    }
}

GLFWwindow* GLFWRenderer::glfwWindow()
{
    return m_window;
}

void GLFWRenderer::frame(float deltaTimeSec)
{
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        m_running = false;
    }
    m_frameUpdate(deltaTimeSec);
    m_frameRender();
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    // sync
    swapBuffers();
    glfwPollEvents();
}

Device* GLFWRenderer::device() const
{
    return m_device;
}
} // namespace backend