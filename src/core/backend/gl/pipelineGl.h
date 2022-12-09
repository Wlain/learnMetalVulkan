//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERGL_H
#define LEARNMETALVULKAN_SHADERGL_H
#include "descriptorSetGl.h"
#include "depthStencilStateGl.h"
#include "glCommonDefine.h"
#include "pipeline.h"

namespace backend
{
class PipelineGL : public Pipeline
{
public:
    explicit PipelineGL(Device* handle);
    ~PipelineGL() override;
    void build() override;
    void setTopology(Topology topology) override;
    void setAttributeDescription(const std::vector<AttributeDescription>& attributeDescriptions) override;
    void setProgram(std::string_view vertShader, std::string_view fragSource) override;
    void setDescriptorSet(const std::shared_ptr<DescriptorSetGl>& descriptorSet);
    void setDepthStencilState(const std::shared_ptr<DepthStencilStateGl>& depthStencilState);
    const std::map<int32_t, DescriptorImageInfo>& imageInfos() const;
    const std::shared_ptr<DepthStencilStateGl>& depthStencilState() const;
    GLuint program() const;
    GLuint vao() const;

private:
    GLuint compileShader(std::string_view resource, GLenum type);

private:
    GLuint m_program{ 0 };
    GLuint m_vao{ 0 };
    std::map<int32_t, DescriptorImageInfo> m_imageInfos;
    std::shared_ptr<DepthStencilStateGl> m_depthStencilState;
};
} // namespace backend

#endif // LEARNMETALVULKAN_SHADERGL_H
