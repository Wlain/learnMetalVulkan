//
// Created by cwb on 2022/9/8.
//

#include "pipelineMtl.h"

#include "commonMacro.h"
#include "glm/glm.hpp"
namespace backend
{
PipelineMtl::PipelineMtl(Device* handle) :
    Pipeline(handle)
{
    m_deviceMtl = static_cast<DeviceMtl*>(handle);
    m_gpu = m_deviceMtl->gpu();
}

void PipelineMtl::setProgram(std::string_view vertShader, std::string_view fragSource)
{
    using NS::StringEncoding::UTF8StringEncoding;
    auto spv = Pipeline::getSpvFromGLSL(vertShader, fragSource);
    auto vertSrc = Pipeline::getMslShaderFromSpv(spv.first);
    auto fragSrc = Pipeline::getMslShaderFromSpv(spv.second);
    LOG_INFO("vertSrc:{}", vertSrc);
    LOG_INFO("fragSrc:{}", fragSrc);
    NS::Error* error = nullptr;
    MTL::Library* vertLibrary = m_gpu->newLibrary(NS::String::string(vertSrc.c_str(), UTF8StringEncoding), nullptr, &error);
    if (!vertLibrary)
    {
        LOG_ERROR("{}", error->localizedDescription()->utf8String());
        ASSERT(0);
    }
    MTL::Library* fragLibrary = m_gpu->newLibrary(NS::String::string(fragSrc.c_str(), UTF8StringEncoding), nullptr, &error);
    if (!fragLibrary)
    {
        LOG_ERROR("{}", error->localizedDescription()->utf8String());
        ASSERT(0);
    }
    auto* vertFunc = vertLibrary->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
    auto* fragFunc = fragLibrary->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));
    auto* descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    auto* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    // pos
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    // color
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat4);
    vertexDescriptor->attributes()->object(1)->setOffset(16);
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    // layout
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(glm::vec4) * 2);
    descriptor->setVertexFunction(vertFunc);
    descriptor->setFragmentFunction(fragFunc);
    descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
    descriptor->setVertexDescriptor(vertexDescriptor);
    m_pipelineState = m_gpu->newRenderPipelineState(descriptor, &error);
    if (!m_pipelineState)
    {
        LOG_ERROR("{}", error->localizedDescription()->utf8String());
        ASSERT(0);
    }
    vertFunc->release();
    fragFunc->release();
    descriptor->release();
    vertLibrary->release();
    fragLibrary->release();
}

void PipelineMtl::build()
{
    Pipeline::build();
}

MTL::RenderPipelineState* PipelineMtl::pipelineState() const
{
    return m_pipelineState;
}

} // namespace backend