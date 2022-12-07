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
    m_deviceMtl = dynamic_cast<DeviceMtl*>(handle);
    m_gpu = m_deviceMtl->gpu();
}

PipelineMtl::~PipelineMtl()
{
    m_vertFunc->release();
    m_fragFunc->release();
    m_descriptor->release();
    m_vertLibrary->release();
    m_fragLibrary->release();
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
    m_vertLibrary = m_gpu->newLibrary(NS::String::string(vertSrc.c_str(), UTF8StringEncoding), nullptr, &error);
    if (!m_vertLibrary)
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
    m_vertFunc = m_vertLibrary->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
    m_fragFunc = fragLibrary->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));
}

void PipelineMtl::build()
{
    m_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    NS::Error* error = nullptr;
    m_descriptor->setVertexFunction(m_vertFunc);
    m_descriptor->setFragmentFunction(m_fragFunc);
    m_descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
    m_descriptor->setVertexDescriptor(m_vertexDescriptor);
    // depth
    m_descriptor->setDepthAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float);
    m_pipelineState = m_gpu->newRenderPipelineState(m_descriptor, &error);
    if (!m_pipelineState)
    {
        LOG_ERROR("{}", error->localizedDescription()->utf8String());
        ASSERT(0);
    }
}

MTL::RenderPipelineState* PipelineMtl::pipelineState() const
{
    return m_pipelineState;
}

static MTL::VertexFormat getFormatMtl(const Format& format)
{
    MTL::VertexFormat result{};
    switch (format)
    {
    case Format::Float16:
        break;
    case Format::Float32:
        result = MTL::VertexFormatFloat4;
        break;
    case Format::Unknown:
    default:
        break;
    }
    return result;
}

void PipelineMtl::setAttributeDescription(const std::vector<AttributeDescription>& attributeDescriptions)
{
    m_vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    // layout
    m_vertexDescriptor->layouts()->object(0)->setStride(attributeDescriptions[0].stride);
    for (size_t i = 0; i < attributeDescriptions.size(); ++i)
    {
        m_vertexDescriptor->attributes()->object(i)->setFormat(getFormatMtl(attributeDescriptions[i].format));
        m_vertexDescriptor->attributes()->object(i)->setOffset(attributeDescriptions[i].offset);
        m_vertexDescriptor->attributes()->object(i)->setBufferIndex(attributeDescriptions[i].binding);
    }
}
} // namespace backend
