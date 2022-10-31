#version 310 es
precision highp float;

layout (location = 0) in vec4 aPos;

layout(binding = 2) uniform VertMVPMatrixUBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos.xyz, 1.0);
}