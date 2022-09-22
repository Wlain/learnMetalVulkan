//
// Created by cwb on 2022/9/22.
//

#include "textureMtl.h"

#include "commonMacro.h"
#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <utils/utils.h>

namespace backend
{
TextureMTL::TextureMTL(Device* device) :
    Texture(device)
{
    m_deviceMtl = dynamic_cast<DeviceMtl*>(device);
}

TextureMTL::~TextureMTL() = default;

bool TextureMTL::createWithRGBAData(const char* data, int width, int height)
{
    return true;
}

bool TextureMTL::createWithFileName(std::string_view filename, bool premultiplyAlpha)
{
    int desireComp = STBI_rgb_alpha;
    stbi_set_unpremultiply_on_load(premultiplyAlpha);
    auto pixelsData = getFileContents(filename.data());
    auto* data = (char*)stbi_load_from_memory((stbi_uc const*)pixelsData.data(), (int)pixelsData.size(), &m_info.width, &m_info.height, &m_info.channels, desireComp);
    bool isPOT = !isPowerOfTwo(m_info.width) || !isPowerOfTwo(m_info.height);
    if (!isPOT && m_info.filterSampling)
    {
        LOG_INFO("Texture {} is not power of two (was {} x {}) filter sampling", filename, m_info.width, m_info.height);
        m_info.filterSampling = false;
    }
    if (isPOT && m_info.generateMipmap)
    {
        LOG_INFO("Texture {} is not power of two (was {} x {}) mipmapping disabled ", filename, m_info.width, m_info.height);
        m_info.generateMipmap = false;
    }
    auto* textureDesc = MTL::TextureDescriptor::alloc()->init();
    textureDesc->setWidth(m_info.width);
    textureDesc->setHeight(m_info.height);
    textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
    textureDesc->setTextureType(MTL::TextureType2D);
    textureDesc->setStorageMode(MTL::StorageModeManaged);
    textureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);
    m_textureHandle = m_deviceMtl->gpu()->newTexture(textureDesc);
    m_textureHandle->replaceRegion(MTL::Region(0, 0, 0, m_info.width, m_info.height, 1), 0, data, m_info.width * 4);
    textureDesc->release();
    stbi_image_free((void*)data);
    return true;
}

MTL::Texture* TextureMTL::handle() const
{
    return m_textureHandle;
}
} // namespace backend