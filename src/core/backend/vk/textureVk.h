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
    const vk::Image& image() const;
    const vk::DeviceMemory& deviceMemory() const;
    const vk::ImageView& imageView() const;
    vk::ImageLayout imageLayout() const;
    const vk::Sampler& sampler() const;

private:
    void updateDescriptor();
    std::pair<vk::Buffer, vk::DeviceMemory> createBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    std::pair<vk::Image, vk::DeviceMemory> createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                       vk::MemoryPropertyFlags properties);
    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
    vk::ImageView createImageView(vk::Image image, vk::Format format);
    vk::Sampler createSampler();

private:
    vk::Image m_image;
    vk::DeviceMemory m_deviceMemory;
    vk::ImageView m_imageView;
    vk::ImageLayout m_imageLayout;
    vk::DescriptorImageInfo m_descriptor;
    vk::Sampler m_sampler;
    DeviceVK* m_deviceVk{ nullptr };
    vk::Device m_device;
};
} // namespace backend
#endif // LEARNMETALVULKAN_TEXTUREVK_H
