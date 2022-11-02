#version 310 es
precision highp float;
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vFragPos;
layout(location = 0) out vec4 fragColor;
layout(binding = 3) uniform FragUniformBufferObject
{
    vec4 lightPos;
    vec4 lightColor;
    vec4 objectColor;
} fragUbo;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * fragUbo.lightColor.rgb;
    // diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(fragUbo.lightPos.xyz - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * fragUbo.lightColor.rgb;
    vec3 result = (ambient + diffuse) * fragUbo.objectColor.rgb;
    fragColor = vec4(result, 1.0);
}