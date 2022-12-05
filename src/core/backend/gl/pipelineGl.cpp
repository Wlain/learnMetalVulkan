//
// Created by cwb on 2022/9/8.
//

#include "pipelineGl.h"

namespace backend
{
Pipeline::Pipeline() = default;

PipelineGL::PipelineGL(Device* handle)
{
    glDeleteVertexArrays(1, &m_vao);
}

PipelineGL::~PipelineGL()
{
    glDeleteProgram(m_program);
}

// 获取当前shader编译信息
void logCurrentCompileInfo(GLuint& shader, GLenum type)
{
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize > 0)
    {
        std::vector<char> errorLog((unsigned long)logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, errorLog.data());
        std::string typeStr;
        switch (type)
        {
        case GL_FRAGMENT_SHADER:
            typeStr = "Fragment shader";
            break;
        case GL_VERTEX_SHADER:
            typeStr = "Vertex shader";
            break;
        case GL_GEOMETRY_SHADER:
            typeStr = "Geometry shader";
            break;
        case GL_TESS_CONTROL_SHADER:
            typeStr = "Tessellation control shader";
            break;
        case GL_TESS_EVALUATION_SHADER:
            typeStr = "Tessellation eval shader";
            break;
        default:
            typeStr = std::string("Unknown error type: ") + std::to_string(type);
            break;
        }
        LOG_ERROR("Shader compile error in {}: {}", typeStr.c_str(), errorLog.data());
    }
}

GLuint PipelineGL::compileShader(std::string_view shaderSource, GLenum type)
{
    auto shader = glCreateShader(type);
    ASSERT(shader != 0);
    const auto* stringPtr = shaderSource.data();
    auto length = (GLint)shaderSource.size();
    glShaderSource(shader, 1, &stringPtr, &length);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    logCurrentCompileInfo(shader, type);
    return shader;
}

void PipelineGL::setProgram(std::string_view vertShader, std::string_view fragSource)
{
    auto spv = Pipeline::getSpvFromGLSL(vertShader, fragSource);
    auto vertSrc = Pipeline::getGlShaderFromSpv(spv.first);
    auto fragSrc = Pipeline::getGlShaderFromSpv(spv.second);
    LOG_INFO("vertSrc:{}", vertSrc);
    LOG_INFO("fragSrc:{}", fragSrc);
    auto vertexShader = compileShader(vertSrc, GL_VERTEX_SHADER);
    auto fragmentShader = compileShader(fragSrc, GL_FRAGMENT_SHADER);
    // link shaders
    m_program = glCreateProgram();
    ASSERT(m_program != 0);
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);
    // check for linking errors
    auto success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        auto logSize = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logSize);
        if (logSize > 0)
        {
            std::vector<char> errorLog((unsigned long)logSize);
            glGetProgramInfoLog(m_program, 512, nullptr, errorLog.data());
            LOG_ERROR("ERROR::SHADER::PROGRAM::LINKING_FAILED: {}", errorLog.data());
        }
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(0);
}

void PipelineGL::build()
{
}

GLuint PipelineGL::program() const
{
    return m_program;
}

void PipelineGL::setTopology(Topology topology)
{
}

GLenum getFormatGl(const Format& format)
{
    GLenum result{};
    switch (format)
    {
    case Format::Float16:
        break;
    case Format::Float32:
        result = GL_FLOAT;
        break;
    case Format::Unknown:
    default:
        break;
    }
    return result;
}

void PipelineGL::setAttributeDescription(const std::vector<AttributeDescription>& attributeDescriptions)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    // set up vertex data (and buffer(s)) and configure vertex attributes
    for (const auto& attribute : attributeDescriptions)
    {
        glEnableVertexAttribArray(attribute.location);
        glVertexAttribPointer(attribute.location, attribute.components, getFormatGl(attribute.format), GL_FALSE, attribute.stride, (void*)attribute.offset);
    }
}

GLuint PipelineGL::vao() const
{
    return m_vao;
}

void PipelineGL::setDescriptorSet(const std::shared_ptr<DescriptorSetGl>& descriptorSet)
{
    for (const auto& shaderResource : descriptorSet->shaderResources())
    {
        auto uboIndex = glGetUniformBlockIndex(m_program, shaderResource.name.c_str());
        glUniformBlockBinding(m_program, uboIndex, shaderResource.binding);
        // define the range of the buffer that links to a uniform binding point
        auto bufferInfos = descriptorSet->bufferInfos();
        if (bufferInfos.find(shaderResource.binding) != descriptorSet->bufferInfos().end())
        {
            auto bufferType = bufferInfos[shaderResource.binding].bufferType;
            auto buffer = bufferInfos[shaderResource.binding].buffer;
            auto offset = bufferInfos[shaderResource.binding].offset;
            auto range = bufferInfos[shaderResource.binding].range;
            glBindBufferRange(bufferType, shaderResource.binding, buffer, offset, range);
        }
    }
    m_imageInfos = descriptorSet->imageInfos();
    if (!m_imageInfos.empty())
    {
        glUseProgram(m_program);
        for (const auto& image : m_imageInfos)
        {
            glUniform1i(glGetUniformLocation(m_program, image.second.name.c_str()), image.first);
        }
        glUseProgram(0);
    }
}

const std::map<int32_t, DescriptorImageInfo>& PipelineGL::imageInfos() const
{
    return m_imageInfos;
}

} // namespace backend
