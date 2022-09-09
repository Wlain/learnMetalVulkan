//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERMTL_H
#define LEARNMETALVULKAN_SHADERMTL_H
#include "pipeline.h"

namespace backend
{
class PipelineMtl : public Pipeline
{
public:
    void setProgram(std::string_view vertShader, std::string_view fragSource) override;
    void build() override;
    ~PipelineMtl() override = default;
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERMTL_H
