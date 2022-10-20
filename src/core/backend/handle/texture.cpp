//
// Created by cwb on 2022/9/22.
//

#include "texture.h"

namespace backend
{
Texture::Texture(Device* device)
{
}

Texture::~Texture() = default;

bool Texture::createWithRGBAData(const char* data, int width, int height)
{
    return false;
}

bool Texture::createWithFileName(std::string_view filename, bool premultiplyAlpha)
{
    return false;
}

bool Texture::createDepthTexture(int width, int height, Texture::DepthPrecision precision)
{
    return false;
}

} // namespace backend