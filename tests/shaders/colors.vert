#version 310 es
precision highp float;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aTexCoord;
layout(location = 0) out vec2 vTexCoord;

layout(binding = 2) uniform VertUniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos.xyz, 1.0);
}