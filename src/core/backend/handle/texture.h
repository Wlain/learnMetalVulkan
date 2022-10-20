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

    enum class DepthPrecision
    {
        I16,          // 16 bit integer
        I24,          // 24 bit integer
        I32,          // 32 bit integer
        F32,          // 32 bit float
        I24_STENCIL8, // 24 bit integer 8 bit stencil
        F32_STENCIL8, // 32 bit float 8 bit stencil
        STENCIL8,     // 8 bit stencil
        None
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
    virtual bool createDepthTexture(int width, int height, DepthPrecision precision);

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
