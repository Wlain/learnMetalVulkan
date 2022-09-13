//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_HANDLEVK_H
#define LEARNMETALVULKAN_HANDLEVK_H
#include "device.h"
#include "glfwRenderer.h"

#include <vulkan/vulkan.hpp>
namespace backend
{
class DeviceVK : public Device
{
public:
    struct SharingModeUtil
    {
        vk::SharingMode sharingMode;
        uint32_t familyIndicesCount;
        uint32_t* familyIndicesDataPtr;
    };

public:
    explicit DeviceVK(const Info& info);
    ~DeviceVK() override;
    void init() override;

    const vk::Instance& instance() const;
    const vk::PhysicalDevice& gpu() const;
    const vk::Device& handle() const;
    const vk::SwapchainKHR& swapChain() const;
    const vk::SurfaceKHR& surface() const;
    const std::vector<uint32_t>& uniqueQueueFamilyIndices() const;
    const std::vector<vk::ImageView>& imageViews() const;

private:
    void initDevice();
    void initInstance();
    void initPhysicalDevice();
    void initDebugger();
    void initSurface();
    void creatSwapChain();

private:
    vk::Instance m_instance;
    vk::PhysicalDevice m_gpu;
    vk::Device m_device;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapChain;
    vk::DebugUtilsMessengerEXT m_messenger;
    std::vector<vk::ImageView> m_imageViews;
    std::vector<uint32_t> m_uniqueQueueFamilyIndices;
};
} // namespace backend

#endif // LEARNMETALVULKAN_HANDLEVK_H
