#version 310 es
precision highp float;
layout(location = 0) out vec4 fragColor;

float near = 0.1;
float far = 100.0;
float linearizeDepth(float depth)
{
    float z = depth * 2.0 -1.0; // [0, 1] -> [-1, 1] to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depth = linearizeDepth(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    fragColor = vec4(vec3(depth), 1.0);
}