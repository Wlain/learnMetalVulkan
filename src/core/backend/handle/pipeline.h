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

enum class Topology
{
    Points,
    LineStrip,
    Lines,
    Triangles,
    TriangleStrip
};

enum class Format
{
    Unknown,
    Float16,
    Float32,
};

enum class InputRate
{
    Vertex,
    Instance,
};

class Pipeline
{
public:
    struct AttributeDescription
    {
        uint32_t binding{ 0 };
        uint32_t stride{ 0 };
        InputRate inputRate{ InputRate::Vertex };
        uint32_t location{ 0 };
        Format format{ Format::Unknown };
        uint32_t offset{ 0 };
    };

public:
    explicit Pipeline(Device* device);
    Pipeline();
    virtual ~Pipeline() = default;
    virtual void setProgram(std::string_view vertShader, std::string_view fragSource) = 0;
    virtual void setTopology(Topology topology);
    virtual void setAttributeDescription(const std::vector<AttributeDescription>& attributeDescriptions);
    virtual void build();

public:
    static std::pair<std::vector<uint32_t>, std::vector<uint32_t>> getSpvFromGLSL(std::string_view vertexSource, std::string_view fragSource);
    static std::string getGlShaderFromSpv(std::vector<uint32_t> shader);
    static std::string getMslShaderFromSpv(std::vector<uint32_t> shader);

protected:
    Device* m_handle{ nullptr };
    std::vector<AttributeDescription> m_attributeDescription;
};
} // namespace backend

#endif // LEARNMETALVULKAN_PIPELINE_H
