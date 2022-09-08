//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADER_H
#define LEARNMETALVULKAN_SHADER_H
#include <string_view>
#include <vector>

class Shader
{
public:
    Shader();
    virtual ~Shader() = default;
    static std::pair<std::vector<uint32_t>, std::vector<uint32_t>> getSpvFromGLSL(std::string_view vertexSource, std::string_view fragSource);
    static std::string getGlShaderFromSpv(std::vector<uint32_t> shader);
    static std::string getMslShaderFromSpv(std::vector<uint32_t> shader);
};

#endif // LEARNMETALVULKAN_SHADER_H
