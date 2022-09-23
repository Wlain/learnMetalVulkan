//
// Created by cwb on 2022/9/22.
//

#include "textureVk.h"

#include <stb/stb_image.h>

namespace backend
{
TextureVK::~TextureVK() = default;
bool TextureVK::createWithRGBAData(const char* data, int width, int height)
{
    return Texture::createWithRGBAData(data, width, height);
}

bool TextureVK::createWithFileName(std::string_view filename, bool premultiplyAlpha)
{
    return Texture::createWithFileName(filename, premultiplyAlpha);
}
} // namespace backend
