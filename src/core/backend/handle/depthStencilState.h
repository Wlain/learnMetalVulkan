//
// Created by william on 2022/12/6.
//

#ifndef LEARNMETALVULKAN_DEPTHSTENCILSTATE_H
#define LEARNMETALVULKAN_DEPTHSTENCILSTATE_H
#include "device.h"

namespace backend
{
enum class CompareOp : uint8_t
{
    Never,          // 不通过（输入的值不取代参考值）
    Less,           // 如果输入的值小于参考值，则通过
    Equal,          // 如果输入的值等于参考值，则通过
    LessOrEqual,    // 如果输入的值小于或等于参考值，则通过
    Greater,        // 如果输入的值大于参考值，则通过
    NotEqual,       // 如果输入的值不等于参考值，则通过
    GreaterOrEqual, // 如果输入的值大于或等于参考值，则通过
    Always,         // 总是通过（输入的值取代参考值）
};

enum class StencilOp : uint8_t
{
    Keep,              // 保持当前储存的模板值
    Zero,              // 将模板值设置为0
    Replace,           // 将模板值设置为StencilFunc函数设置的ref值
    IncrementAndClamp, // 如果模板值小于最大值则将模板值加1
    DecrementAndClamp, // 如果模板值大于最小值则将模板值减1
    Invert,            // 按位翻转当前的模板缓冲值
    IncrementAndWrap,  // 与IncrementAndClamp一样，但如果模板值超过了最大值则归零
    DecrementAndWrap,  // 与DecrementAndClamp一样，但如果模板值小于0则将其设置为最大值
};

// 以下为默认值
struct StencilOpState
{
    StencilOp stencilFailOp{ StencilOp::Keep };      // 模板测试失败时采取的行为
    StencilOp depthStencilPassOp{ StencilOp::Keep }; // 模板测试和深度测试都通过时采取的行为
    StencilOp depthFailOp{ StencilOp::Keep };        // 模板测试通过，但深度测试失败时采取的行为
    CompareOp stencilCompareOp{ CompareOp::Always }; // 模板测试比较函数
    uint32_t compareMask{ UINT32_MAX };              // 对比的掩码值
    uint32_t writeMask{ UINT32_MAX };                // 写入值
    uint32_t reference{ UINT32_MAX };                // 参考值
};

class DepthStencilState
{
public:
    explicit DepthStencilState(Device* device);
    virtual ~DepthStencilState();

public:
    virtual void setDepthCompareOp(CompareOp depthCompareOp);
    virtual void setStencilTestEnable(bool stencilTestEnable);
    virtual void setFront(const StencilOpState& front);
    virtual void setBack(const StencilOpState& back);
    virtual void setDepthTestEnable(bool depthTestEnable);
    virtual void setDepthWriteEnable(bool depthWriteEnable);
    virtual void setDepthBoundsTestEnable(bool depthBoundsTestEnable);
    [[nodiscard]] CompareOp depthCompareOp() const;
    [[nodiscard]] const StencilOpState& front() const;
    [[nodiscard]] const StencilOpState& back() const;
    [[nodiscard]] bool depthTestEnable() const;
    [[nodiscard]] bool depthWriteEnable() const;
    [[nodiscard]] bool depthBoundsTestEnable() const;
    [[nodiscard]] bool stencilTestEnable() const;

protected:
    CompareOp m_depthCompareOp = CompareOp::Always; // 深度比较函数
    StencilOpState m_front{};                       // 正面状态
    StencilOpState m_back{};                        // 背面状态
    bool m_depthTestEnable = false;                 // 是否打开深度测试
    bool m_depthWriteEnable = false;                // 是否写入深度
    bool m_depthBoundsTestEnable = false;           // 是否打开深度边界测试
    bool m_stencilTestEnable = false;               // 是否打开模板测试
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEPTHSTENCILSTATE_H
