//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_PIPELINE_H
#define LEARNMETALVULKAN_PIPELINE_H
#include <string_view>
#include <vector>

namespace backend
{
class Device;
class Pipeline
{
public:
    explicit Pipeline(Device* handle);
    Pipeline();
    virtual ~Pipeline() = default;
    virtual void setProgram(std::string_view vertShader, std::string_view fragSource) = 0;
    virtual void build();

public:
    static std::pair<std::vector<uint32_t>, std::vector<uint32_t>> getSpvFromGLSL(std::string_view vertexSource, std::string_view fragSource);
    static std::string getGlShaderFromSpv(std::vector<uint32_t> shader);
    static std::string getMslShaderFromSpv(std::vector<uint32_t> shader);

protected:
    Device* m_handle{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_PIPELINE_H
