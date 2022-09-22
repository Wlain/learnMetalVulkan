//
// Created by cwb on 2022/9/22.
//

#include "textureVk.h"
namespace backend
{
TextureVK::~TextureVK() = default;
bool TextureVK::createWithRGBAData(const char* data, int width, int height)
{
    return Texture::createWithRGBAData(data, width, height);
}
} // namespace backend
