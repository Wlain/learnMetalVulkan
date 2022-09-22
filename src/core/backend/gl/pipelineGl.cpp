//
// Created by cwb on 2022/9/8.
//

#include "pipelineGl.h"

namespace backend
{
Pipeline::Pipeline() = default;

PipelineGL::PipelineGL(Device* handle)
{
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
    Pipeline::build();
}

GLuint PipelineGL::program() const
{
    return m_program;
}

} // namespace backend
