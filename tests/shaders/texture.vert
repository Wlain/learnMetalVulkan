#version 310 es
precision highp float;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aTexCoord;
layout(location = 0) out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord.xy;
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}