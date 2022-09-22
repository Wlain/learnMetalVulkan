//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTUREMTL_H
#define LEARNMETALVULKAN_TEXTUREMTL_H
#include "texture.h"
namespace backend
{
class TextureMTL : public Texture
{
public:
    ~TextureMTL() override;
    bool createWithRGBAData(const char* data, int width, int height) override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_TEXTUREMTL_H
