//
// Created by cwb on 2022/9/8.
//

#include "shader.h"

#include "commonMacro.h"
#include "resourceLimits.h"

#include <glslang/SPIRV/GlslangToSpv.h>
Shader::Shader()
{
    LOG_INFO("SpirvGeneratorVersion:{}", glslang::GetSpirvGeneratorVersion());
}

std::pair<std::vector<uint32_t>, std::vector<uint32_t>> Shader::getSpvFromGLSL(std::string_view vertexSource, std::string_view fragSource)
{
    static bool glslangInited = false;
    if (!glslangInited)
    {
        glslang::InitializeProcess();
        glslangInited = true;
    }
    std::array<const char*, 2> strings = { { vertexSource.data(), fragSource.data() } };
    std::array<int, 2> lengths = {
        { static_cast<int>(vertexSource.length()), static_cast<int>(fragSource.length()) }
    };

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
    glslang::TShader vertexShader(EShLangVertex);
    vertexShader.setStringsWithLengths(&strings[0], &lengths[0], 1);
    vertexShader.setEntryPoint("main");
    bool vertexResult = vertexShader.parse(&glslang::DefaultTBuiltInResource, 450, ECoreProfile, false, false, messages);
    if (!vertexResult)
    {
        LOG_ERROR("Internal error parsing Vulkan vertex shader:{}, {}", vertexShader.getInfoLog(), vertexShader.getInfoDebugLog());
    }

    glslang::TShader fragmentShader(EShLangFragment);
    fragmentShader.setStringsWithLengths(&strings[1], &lengths[1], 1);
    fragmentShader.setEntryPoint("main");
    bool fragmentResult = fragmentShader.parse(&glslang::DefaultTBuiltInResource, 450, ECoreProfile,
                                               false, false, messages);
    if (!fragmentResult)
    {
        LOG_ERROR("Internal error parsing Vulkan fragment shader:{}, {}", fragmentShader.getInfoLog(), fragmentShader.getInfoDebugLog());
    }
    glslang::TProgram program;
    program.addShader(&vertexShader);
    program.addShader(&fragmentShader);
    bool linkResult = program.link(messages);
    if (!linkResult)
    {
        LOG_ERROR("Internal error linking Vulkan shaders:{}", program.getInfoLog());
    }
    glslang::TIntermediate* vertexStage = program.getIntermediate(EShLangVertex);
    glslang::TIntermediate* fragmentStage = program.getIntermediate(EShLangFragment);
    std::vector<uint32_t> spvVs, spvFs;
    glslang::GlslangToSpv(*vertexStage, spvVs);
    glslang::GlslangToSpv(*fragmentStage, spvFs);
    return { spvVs, spvFs };
}

std::pair<const char*, const char*> Shader::getMtlShaderFromSpv(std::vector<uint32_t>, std::vector<uint32_t>)
{
    return {};
}

std::pair<const char*, const char*> Shader::getGlShaderFromSpv(std::vector<uint32_t>, std::vector<uint32_t>)
{
    return {};
}
