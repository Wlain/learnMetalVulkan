//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERMTL_H
#define LEARNMETALVULKAN_SHADERMTL_H
#include "deviceMtl.h"
#include "pipeline.h"

namespace backend
{
class PipelineMtl : public Pipeline
{
public:
    explicit PipelineMtl(Device* handle);
    void setProgram(std::string_view vertShader, std::string_view fragSource) override;
    void build() override;
    ~PipelineMtl() override;
    MTL::RenderPipelineState* pipelineState() const;
    void setVertexDescriptor(MTL::VertexDescriptor* mVertexDescriptor);

private:
    DeviceMtl* m_deviceMtl{ nullptr };
    MTL::Device* m_gpu{ nullptr };
    MTL::RenderPipelineState* m_pipelineState{ nullptr };
    MTL::Function* m_vertFunc{};
    MTL::Library* m_vertLibrary{};
    MTL::Function* m_fragFunc{};
    MTL::Library* m_fragLibrary{};
    MTL::RenderPipelineDescriptor* m_descriptor{};
    MTL::VertexDescriptor* m_vertexDescriptor{};
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERMTL_H
