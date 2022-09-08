//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_PIPELINE_H
#define LEARNMETALVULKAN_PIPELINE_H
#include <string>
#include <string_view>
#include <vector>
namespace backend
{
class Device;
class Pipeline
{
public:
    explicit Pipeline(Device* handle);
    Pipeline(std::string_view vert, std::string_view frag);
    Pipeline();
    virtual ~Pipeline() = default;
    virtual void initialize();

public:
    static std::pair<std::vector<uint32_t>, std::vector<uint32_t>> getSpvFromGLSL(std::string_view vertexSource, std::string_view fragSource);
    static std::string getGlShaderFromSpv(std::vector<uint32_t> shader);
    static std::string getMslShaderFromSpv(std::vector<uint32_t> shader);

protected:
    Device* m_handle{ nullptr };
    std::string m_vertShader;
    std::string m_fragShader;
};
} // namespace backend

#endif // LEARNMETALVULKAN_PIPELINE_H
