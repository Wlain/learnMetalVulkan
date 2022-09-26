//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTUREMTL_H
#define LEARNMETALVULKAN_TEXTUREMTL_H
#include "deviceMtl.h"
#include "mtlCommonDefine.h"
#include "texture.h"

namespace backend
{
class TextureMTL : public Texture
{
public:
    TextureMTL(Device* device);
    ~TextureMTL() override;
    bool createWithRGBAData(const char* data, int width, int height) override;
    bool createWithFileName(std::string_view filename, bool premultiplyAlpha) override;
    MTL::Texture* handle() const;

public:
    MTL::Texture* m_textureHandle{ nullptr };
    DeviceMtl* m_deviceMtl{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_TEXTUREMTL_H
