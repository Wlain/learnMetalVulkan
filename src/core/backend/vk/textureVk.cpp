//
// Created by cwb on 2022/9/22.
//

#include "textureVk.h"

#include "utils/utils.h"
#include "vkCommonDefine.h"

#include <stb/stb_image.h>
namespace backend
{
TextureVK::TextureVK(Device* device) :
    Texture(device)
{
    m_deviceVk = static_cast<DeviceVK*>(device);
}

TextureVK::~TextureVK()
{
    auto device = m_deviceVk->handle();
    device.destroy(m_imageView);
    device.destroy(m_image);
    if (m_sampler)
    {
        device.destroy(m_sampler);
    }
    device.free(m_deviceMemory);
}

bool TextureVK::createWithRGBAData(const char* data, int width, int height)
{
    return Texture::createWithRGBAData(data, width, height);
}

bool TextureVK::createWithFileName(std::string_view filename, bool premultiplyAlpha)
{
    auto device = m_deviceVk->handle();
    auto gpu = m_deviceVk->gpu();
    int desireComp = STBI_rgb_alpha;
    stbi_set_unpremultiply_on_load(premultiplyAlpha);
    stbi_set_flip_vertically_on_load(true);
    auto pixelsData = getFileContents(filename.data());
    auto* data = (char*)stbi_load_from_memory((stbi_uc const*)pixelsData.data(), (int)pixelsData.size(), &m_info.width, &m_info.height, &m_info.channels, desireComp);
    stbi_set_flip_vertically_on_load(false);
    m_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_info.width, m_info.height)))) + 1;
    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    // We prefer using staging to copy the texture data to a device local optimal image
    bool useStaging = false;
    vk::MemoryAllocateInfo memAllocInfo{};
    vk::MemoryRequirements memReqs{};
    if (useStaging)
    {
        // TODO:Copy data to an optimal tiled image
    }
    else
    {
        vk::Image mappableImage;
        vk::DeviceMemory mappableMemory;
        auto imageCreateInfo = vk::ImageCreateInfo{
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = { static_cast<uint32_t>(m_info.width), static_cast<uint32_t>(m_info.height), 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eLinear,
            .usage = vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::ePreinitialized
        };
        mappableImage = device.createImage(imageCreateInfo);
        // 获取图像的内存参数，如大小和对齐方式
        memReqs = device.getImageMemoryRequirements(mappableImage);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_deviceVk->findMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        mappableMemory = device.allocateMemory(memAllocInfo);
        device.bindImageMemory(mappableImage, mappableMemory, 0);
        // Map image memory
        std::size_t imageSize = m_info.width * m_info.height * 4;
        auto* pData = (device.mapMemory(mappableMemory, {}, memReqs.size, {}));
        memcpy(pData, data, imageSize);
        device.unmapMemory(mappableMemory);
        // Linear tiled images don't need to be staged and can be directly used as textures
        m_image = mappableImage;
        m_deviceMemory = mappableMemory;
        // Setup image memory barrier transfer image to shader read layout
        auto commandPool = m_deviceVk->commandPool();
        auto allocInfo = vk::CommandBufferAllocateInfo{
            .commandPool = commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto commandBuffers = device.allocateCommandBuffers(allocInfo);

    }
    stbi_image_free((void*)data);
    return true;
}

void TextureVK::updateDescriptor()
{
    m_descriptor.sampler = m_sampler;
    m_descriptor.imageLayout = m_imageLayout;
    m_descriptor.imageView = m_imageView;
}

} // namespace backend
