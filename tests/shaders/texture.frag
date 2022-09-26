#version 310 es
precision highp float;
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D inputTexture;

void main()
{
    fragColor = texture(inputTexture, vTexCoord);
}