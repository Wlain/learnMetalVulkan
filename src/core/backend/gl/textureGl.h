//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTUREGL_H
#define LEARNMETALVULKAN_TEXTUREGL_H
#include "glCommonDefine.h"
#include "texture.h"

namespace backend
{
class TextureGL : public Texture
{
public:
    using Texture::Texture;
    ~TextureGL() override;

public:
    bool createWithRGBAData(const char* data, int width, int height) override;
    bool createWithFileName(std::string_view filename, bool premultiplyAlpha) override;
    void updateTextureSampler(bool filterSampling, Wrap warp) const;
    bool createDepthTexture(int width, int height, DepthPrecision precision) override;
    GLuint handle() const;
    GLuint target() const;

private:
    GLuint m_textureHandle;
};
} // namespace backend

#endif // LEARNMETALVULKAN_TEXTUREGL_H
