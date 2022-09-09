//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERGL_H
#define LEARNMETALVULKAN_SHADERGL_H
#include "pipeline.h"
namespace backend
{
class PipelineGL : public Pipeline
{
public:
    explicit PipelineGL(Device* handle);
    ~PipelineGL() override = default;
    void build() override;
    void setProgram(std::string_view vertShader, std::string_view fragSource) override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERGL_H
