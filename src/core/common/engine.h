//
// Created by cwb on 2022/9/5.
//

#ifndef LEARNMETALVULKAN_ENGINE_H
#define LEARNMETALVULKAN_ENGINE_H

#include "commonHandle.h"
#include "glfwRenderer.h"
namespace backend
{
class Engine
{
public:
    Engine(GLFWRenderer& renderer, std::string_view title);
    virtual ~Engine() = default;

public:
    void setEffect(const std::shared_ptr<EffectBase>& effect);
    void run();

private:
    GLFWRenderer& m_renderer;
    std::shared_ptr<CommonHandle> m_handle;
};
} // namespace backend

#endif // LEARNMETALVULKAN_ENGINE_H
