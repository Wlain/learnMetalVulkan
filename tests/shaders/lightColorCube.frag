#version 310 es
precision highp float;
layout(location = 0) out vec4 fragColor;

layout(binding = 3) uniform FragUniformBufferObject
{
    vec4 lightColor;
} fragUbo;

void main()
{
    fragColor = vec4(fragUbo.lightColor.rgb, 1.0);
}