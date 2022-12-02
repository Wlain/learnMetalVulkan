#version 310 es
precision highp float;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoords;
layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vFragPos;
layout(location = 2) out vec2 vTexCoords;

layout(binding = 2) uniform VertMVPMatrixUBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    vFragPos = vec3(ubo.model * vec4(aPos.xyz, 1.0));
    vNormal = aNormal.xyz;
    vTexCoords = aTexCoords.xy;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos.xyz, 1.0);
}