#version 310 es
precision highp float;
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D inputTexture;

void main()
{
    vec4 colorX = vTexCoord.x < 0.5 ? vec4(1.0) : vec4(1.0, 0.0, 0.0, 1.0);
    vec4 colorY = vTexCoord.y < 0.5 ? vec4(1.0) : vec4(0.0, 1.0, 0.0, 1.0);
    fragColor = texture(inputTexture, vTexCoord) * colorX * colorY;
}