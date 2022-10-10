//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_TEXTURE_H
#define LEARNMETALVULKAN_TEXTURE_H
#include "device.h"

#include <cstdint>
#include <string>
namespace backend
{
class Texture
{
public:
    enum class PixelFormat : uint8_t
    {
        RGBA,
        RGB
    };
    enum class SamplerColorspace : uint8_t
    {
        Linear,
        Gamma
    };

    enum class Wrap
    {
        Repeat,
        ClampToEdge,
        Mirror
    };

    struct Info
    {
        PixelFormat format = PixelFormat::RGBA;
        SamplerColorspace colorspace = SamplerColorspace::Linear;
        Wrap warp = Wrap::Repeat;
        int width = 0;
        int height = 0;
        int channels = 0;
        uint32_t layerCount;
        uint32_t mipLevels;
        uint32_t target = 0;
        bool transparent; // 透明纹理
        bool generateMipmap = false;
        bool filterSampling = true;
    };

public:
    Texture(Device* device);
    virtual ~Texture();

public:
    virtual bool createWithFileName(std::string_view filename, bool premultiplyAlpha);
    virtual bool createWithRGBAData(const char* data, int width, int height);
    inline bool isPowerOfTwo(unsigned int x)
    {
        // 例如：8:1000 7:111  8 & 7 == 0
        return ((x != 0) && !(x & (x - 1)));
    }
    inline int width() const { return m_info.width; }
    inline int height() const { return m_info.height; }
    inline bool isFilterSampling() const { return m_info.filterSampling; }
    inline Wrap warpUv() const { return m_info.warp; }

protected:
    Info m_info{};
};
} // namespace backend

#endif // LEARNMETALVULKAN_TEXTURE_H
