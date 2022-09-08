//
// Created by cwb on 2022/9/6.
//
#include "commonHandle.h"
#include "engine.h"
#include "glfwRendererVk.h"

class Window : public EffectBase
{
public:
    using EffectBase::EffectBase;

    void initialize() override
    {
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        // const char kShaderSource[]
        std::string kShaderSource = R"vertexshader(
        #version 450
        #extension GL_ARB_separate_shader_objects : enable
        out gl_PerVertex {
            vec4 gl_Position;
        };
        layout(location = 0) out vec3 fragColor;
        vec2 positions[3] = vec2[](
            vec2(0.0, -0.5),
            vec2(0.5, 0.5),
            vec2(-0.5, 0.5)
        );
        vec3 colors[3] = vec3[](
            vec3(1.0, 0.0, 0.0),
            vec3(0.0, 1.0, 0.0),
            vec3(0.0, 0.0, 1.0)
        );
        void main() {
            gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
            fragColor = colors[gl_VertexIndex];
        }
        )vertexshader";

        std::string fragmentShader = R"fragmentShader(
        #version 450
        #extension GL_ARB_separate_shader_objects : enable
        layout(location = 0) in vec3 fragColor;
        layout(location = 0) out vec4 outColor;
        void main() {
            outColor = vec4(fragColor, 1.0);
        }
        )fragmentShader";
        m_render->initPipeline(kShaderSource, fragmentShader);
        m_render->initCommandBuffer();
    }

    void render() override
    {
        m_render->commit();
    }

private:
    GLFWRendererVK* m_render{ nullptr };
};

void triangleVk()
{
    GLFWRendererVK renderer;
    Engine engine(renderer, "Vulkan Example Test");
    auto effect = std::make_shared<Window>(&renderer);
    engine.setEffect(effect);
    engine.run();
}