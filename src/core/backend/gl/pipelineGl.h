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
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERGL_H
