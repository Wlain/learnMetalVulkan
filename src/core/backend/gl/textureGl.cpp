//
// Created by cwb on 2022/9/22.
//

#include "textureGl.h"

#include "utils/utils.h"

#include <stb/stb_image.h>

namespace backend
{
TextureGL::~TextureGL()
{
    if (m_textureHandle != 0)
    {
        glDeleteTextures(1, &m_textureHandle);
    }
}

void TextureGL::updateTextureSampler(bool filterSampling, Wrap warp) const
{
    glBindTexture(m_info.target, m_textureHandle);
    auto wrapEnum = warp == Wrap::ClampToEdge ? GL_CLAMP_TO_EDGE : (warp == Wrap::Mirror ? GL_MIRRORED_REPEAT : GL_REPEAT);
    glTexParameteri(m_info.target, GL_TEXTURE_WRAP_S, wrapEnum);
    glTexParameteri(m_info.target, GL_TEXTURE_WRAP_T, wrapEnum);
    GLint minification;
    GLint magnification;
    if (!filterSampling)
    {
        minification = GL_NEAREST;
        magnification = GL_NEAREST;
    }
    else if (m_info.generateMipmap)
    {
        minification = GL_LINEAR_MIPMAP_LINEAR;
        magnification = GL_LINEAR;
    }
    else
    {
        minification = GL_LINEAR;
        magnification = GL_LINEAR;
    }
    glTexParameteri(m_info.target, GL_TEXTURE_MAG_FILTER, magnification);
    glTexParameteri(m_info.target, GL_TEXTURE_MIN_FILTER, minification);
}

bool TextureGL::createWithRGBAData(const char* data, int width, int height)
{
    glGenTextures(1, &m_textureHandle);
    m_info.width = width;
    m_info.height = height;
    m_info.format = PixelFormat::RGBA;
    m_info.target = GL_TEXTURE_2D;
    GLint mipmapLevel = 0;
    GLint internalFormat = m_info.colorspace == SamplerColorspace::Linear ? GL_SRGB_ALPHA : GL_RGBA;
    GLint border = 0;
    GLenum type = GL_UNSIGNED_BYTE;
    glBindTexture(m_info.target, m_textureHandle);
    glTexImage2D(m_info.target, mipmapLevel, internalFormat, m_info.width, m_info.height, border, GL_RGBA, type, data);
    updateTextureSampler(m_info.filterSampling, m_info.warp);
    return true;
}

bool TextureGL::createWithFileName(std::string_view filename, bool premultiplyAlpha)
{
    glGenTextures(1, &m_textureHandle);
    m_info.target = GL_TEXTURE_2D;
    GLint mipmapLevel = 0;
    GLint internalFormat = m_info.colorspace == SamplerColorspace::Gamma ? GL_SRGB_ALPHA : GL_RGBA;
    GLint border = 0;
    GLenum type = GL_UNSIGNED_BYTE;
    int desireComp = STBI_rgb_alpha;
    stbi_set_unpremultiply_on_load(premultiplyAlpha);
    stbi_set_flip_vertically_on_load(true);
    auto pixelsData = getFileContents(filename.data());
    auto* data = (char*)stbi_load_from_memory((stbi_uc const*)pixelsData.data(), (int)pixelsData.size(), &m_info.width, &m_info.height, &m_info.channels, desireComp);
    stbi_set_flip_vertically_on_load(false);
    glBindTexture(m_info.target, m_textureHandle);
    bool isPOT = !isPowerOfTwo(m_info.width) || !isPowerOfTwo(m_info.height);
    if (!isPOT && m_info.filterSampling)
    {
        LOG_INFO("Texture {} is not power of two (was {} x {}) filter sampling", filename, m_info.width, m_info.height);
        m_info.filterSampling = false;
    }
    if (isPOT && m_info.generateMipmap)
    {
        LOG_INFO("Texture {} is not power of two (was {} x {}) mipmapping disabled ", filename, m_info.width, m_info.height);
        m_info.generateMipmap = false;
    }
    checkGlError();
    glTexImage2D(m_info.target, mipmapLevel, internalFormat, m_info.width, m_info.height, border, GL_RGBA, type, data);
    updateTextureSampler(m_info.filterSampling, m_info.warp);
    checkGlError();
    stbi_image_free((void*)data);
    return true;
}

GLuint TextureGL::handle() const
{
    return m_textureHandle;
}
} // namespace backend