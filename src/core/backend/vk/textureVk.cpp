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
    m_deviceVk = dynamic_cast<DeviceVK*>(device);
    m_device = m_deviceVk->handle();
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
    int desireComp = STBI_rgb_alpha;
    stbi_set_unpremultiply_on_load(premultiplyAlpha);
    stbi_set_flip_vertically_on_load(true);
    auto pixelsData = getFileContents(filename.data());
    auto* pixels = (char*)stbi_load_from_memory((stbi_uc const*)pixelsData.data(), (int)pixelsData.size(), &m_info.width, &m_info.height, &m_info.channels, desireComp);
    stbi_set_flip_vertically_on_load(false);
    size_t imageSize = m_info.width * m_info.height * 4;
    m_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_info.width, m_info.height)))) + 1;
    // We prefer using staging to copy the texture data to a device local optimal image
    bool useStaging = true;
    if (useStaging)
    {
        auto [stagingBuffer, stagingBufferMemory] = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        // copy the image data into that device memory
        auto* pData = (device.mapMemory(stagingBufferMemory, {}, imageSize, {}));
        memcpy(pData, pixels, static_cast<std::size_t>(imageSize));
        device.unmapMemory(stagingBufferMemory);
        stbi_image_free((void*)pixels);
        auto [mappableImage, mappableMemory] = createImage(m_info.width, m_info.height, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
    }
    else
    {
        auto [mappableImage, mappableMemory] = createImage(m_info.width, m_info.height, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
        // Map image memory
        std::size_t imageSize = m_info.width * m_info.height * 4;
        auto* pData = (device.mapMemory(mappableMemory, {}, imageSize, {}));
        memcpy(pData, pixels, imageSize);
        device.unmapMemory(mappableMemory);
        stbi_image_free((void*)pixels);
    }
    return true;
}

void TextureVK::updateDescriptor()
{
    m_descriptor.sampler = m_sampler;
    m_descriptor.imageLayout = m_imageLayout;
    m_descriptor.imageView = m_imageView;
}

std::pair<vk::Buffer, vk::DeviceMemory> TextureVK::createBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    auto device = m_deviceVk->handle();
    auto bufferInfo = vk::BufferCreateInfo{
        .size = bufferSize,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    auto buffer = device.createBuffer(bufferInfo);

    auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
    auto allocInfo = vk::MemoryAllocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = m_deviceVk->findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };
    auto bufferMemory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(buffer, bufferMemory, 0);
    return { buffer, bufferMemory };
}

std::pair<vk::Image, vk::DeviceMemory> TextureVK::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::Image mappableImage;
    vk::DeviceMemory mappableMemory;
    vk::MemoryAllocateInfo memAllocInfo{};
    vk::MemoryRequirements memReqs{};
    auto imageCreateInfo = vk::ImageCreateInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::ePreinitialized
    };
    mappableImage = m_device.createImage(imageCreateInfo);
    // 获取图像的内存参数，如大小和对齐方式
    memReqs = m_device.getImageMemoryRequirements(mappableImage);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = m_deviceVk->findMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    mappableMemory = m_device.allocateMemory(memAllocInfo);
    m_device.bindImageMemory(mappableImage, mappableMemory, 0);
    return { mappableImage, mappableMemory };
}

void TextureVK::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    auto commandBuffer = m_deviceVk->beginSingleTimeCommands();
    auto barrier = vk::ImageMemoryBarrier{
        .oldLayout = oldLayout,
        newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        image = image,
        .subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
    m_deviceVk->endSingleTimeCommands(commandBuffer);
}

} // namespace backend
