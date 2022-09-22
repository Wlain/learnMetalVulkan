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
    ~PipelineMtl() override = default;
    MTL::RenderPipelineState* pipelineState() const;

private:
    DeviceMtl* m_deviceMtl{nullptr};
    MTL::Device* m_gpu{ nullptr };
    MTL::RenderPipelineState* m_pipelineState{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERMTL_H
