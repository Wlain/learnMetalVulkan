#version 310 es
precision highp float;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
layout(location = 0) out vec3 vColor;

void main()
{
    vColor = aColor.xyz;
    gl_Position = vec4(aPos.xyx, 1.0);
}