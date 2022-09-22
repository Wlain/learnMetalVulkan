//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_HANDLEMTL_H
#define LEARNMETALVULKAN_HANDLEMTL_H
#include "device.h"
#define GLFW_INCLUDE_NONE
#include "mtlCommonDefine.h"
namespace backend
{
class DeviceMtl : public Device
{
public:
    explicit DeviceMtl(const Info& info);
    ~DeviceMtl() override;
    void init() override;

    CA::MetalLayer* swapChain() const;
    MTL::Device* gpu() const;
    MTL::CommandQueue* queue() const;

private:
    CA::MetalLayer* m_swapChain{ nullptr };
    MTL::Device* m_gpu{ nullptr };
    MTL::CommandQueue* m_queue{ nullptr };
};
} // namespace backend

#endif // LEARNMETALVULKAN_HANDLEMTL_H
