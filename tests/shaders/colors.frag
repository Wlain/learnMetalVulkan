#version 310 es
precision highp float;
layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform FragUniformBufferObject
{
    vec3 lightColor;
    vec3 objectColor;
} ubo;

void main()
{
    fragColor = vec4(ubo.objectColor * ubo.lightColor, 1.0);
}