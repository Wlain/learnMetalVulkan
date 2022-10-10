//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTUREVK_H
#define LEARNMETALVULKAN_TEXTUREVK_H
#include "deviceVk.h"
#include "texture.h"
namespace backend
{
class TextureVK : public Texture
{
public:
    explicit TextureVK(Device* device);
    ~TextureVK() override;
    bool createWithRGBAData(const char* data, int width, int height) override;
    bool createWithFileName(std::string_view filename, bool premultiplyAlpha) override;

public:
    void updateDescriptor();

private:
    vk::Image m_image;
    vk::ImageView m_imageView;
    vk::DeviceMemory m_deviceMemory;
    vk::ImageLayout m_imageLayout;
    vk::DescriptorImageInfo m_descriptor;
    vk::Sampler m_sampler;
    DeviceVK* m_deviceVk{ nullptr };
};
} // namespace backend
#endif // LEARNMETALVULKAN_TEXTUREVK_H
