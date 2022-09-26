//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTUREVK_H
#define LEARNMETALVULKAN_TEXTUREVK_H
#include "texture.h"
namespace backend
{
class TextureVK : public Texture
{
public:
    ~TextureVK() override;
    bool createWithRGBAData(const char* data, int width, int height) override;
    bool createWithFileName(std::string_view filename, bool premultiplyAlpha) override;
};
} // namespace backend
#endif // LEARNMETALVULKAN_TEXTUREVK_H
