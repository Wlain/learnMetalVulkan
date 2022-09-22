//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTUREGL_H
#define LEARNMETALVULKAN_TEXTUREGL_H
#include "texture.h"
namespace backend
{
class TextureGL : public Texture
{
public:
    ~TextureGL() override;
    bool createWithRGBAData(const char* data, int width, int height) override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_TEXTUREGL_H
