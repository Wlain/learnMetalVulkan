#version 310 es
precision highp float;
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec2 vTexCoords;
layout(location = 0) out vec4 fragColor;

struct Material
{
    float shininess;
};

struct Light
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

layout(binding = 3) uniform FragUniformBufferObject
{
    vec4 viewPos;
    Material material;
    Light light;
} fragUbo;

layout(binding = 4) uniform sampler2D diffuseTexture;

layout(binding = 5) uniform sampler2D specularTexture;

void main()
{
    // ambient
    vec3 ambient = fragUbo.light.ambient.rgb * texture(diffuseTexture, vTexCoords).rgb;

    // diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(fragUbo.light.position.xyz - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = fragUbo.light.diffuse.rgb * texture(diffuseTexture, vTexCoords).rgb * diff;

    // specular
    vec3 viewDir = normalize(fragUbo.viewPos.xyz - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), fragUbo.material.shininess);
    vec3 specular = fragUbo.light.specular.rgb * texture(specularTexture, vTexCoords).rgb * spec;
    vec3 result = ambient + diffuse + specular;

    fragColor = vec4(result, 1.0);
}