//
// Created by cwb on 2022/9/22.
//

#include "textureGl.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace backend
{
TextureGL::~TextureGL() = default;

bool TextureGL::createWithRGBAData(const char* data, int width, int height)
{
}
} // namespace backend