//
// Created by cwb on 2022/9/8.
//

#include "shader.h"

#include "commonMacro.h"
#include "resourceLimits.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <utility>

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

std::string Shader::getGlShaderFromSpv(std::vector<uint32_t> shader)
{
    using namespace spirv_cross;
    static CompilerGLSL::Options options;
    CompilerGLSL glsl(std::move(shader));
    options.version = 450;
    options.es = true;
    glsl.set_common_options(options);
    return glsl.compile();
}

std::string Shader::getMslShaderFromSpv(std::vector<uint32_t> shader)
{
    using namespace spirv_cross;
    CompilerMSL msl(shader);
    auto option = msl.get_msl_options();
    option.ios_support_base_vertex_instance = true;
    msl.set_msl_options(option);
    ShaderResources resources = msl.get_shader_resources();
    MSLConstexprSampler sampler;
    sampler.s_address = MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
    sampler.t_address = MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
    sampler.r_address = MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
    sampler.min_filter = MSL_SAMPLER_FILTER_LINEAR;
    sampler.mag_filter = MSL_SAMPLER_FILTER_LINEAR;
    for (auto& resource : resources.sampled_images)
    {
        uint32_t set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        uint32_t binding = msl.get_decoration(resource.id, spv::DecorationBinding);
        msl.remap_constexpr_sampler_by_binding(set, binding, sampler);
    }
    auto entryPoint = msl.get_entry_points_and_stages();
    for (auto& e : entryPoint)
    {
        msl.rename_entry_point(e.name, e.execution_model == spv::ExecutionModelVertex ? "vertexShader" : "fragmentShader", e.execution_model);
    }
    return msl.compile();
}
