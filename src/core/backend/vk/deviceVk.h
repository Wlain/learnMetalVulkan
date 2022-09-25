//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_HANDLEVK_H
#define LEARNMETALVULKAN_HANDLEVK_H
#include "device.h"
#include "glfwRenderer.h"
#define VULKAN_HPP_NO_CONSTRUCTORS // 从 vulkan.hpp 中删除所有结构和联合构造函数
#include <optional>
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

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        [[nodiscard]] inline bool isComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
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
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
    const vk::Queue& graphicsQueue() const;
    const vk::Queue& presentQueue() const;
    const std::vector<vk::Image>& swapchainImages() const;
    const std::vector<vk::ImageView>& swapchainImageViews() const;
    vk::Format swapchainImageFormat() const;
    const vk::Extent2D& swapchainExtent() const;
    QueueFamilyIndices findQueueFamilyIndices(vk::PhysicalDevice gpu);

private:
    void initInstance();
#ifndef NDEBUG
    void initDebugger();
#endif
    void pickPhysicalDevice();
    void initDevice();
    void initSurface();
    void creatSwapChain();
    void createImageViews();
    bool isDeviceSuitable(vk::PhysicalDevice gpu);
    [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice gpu) const;
    void cleanupSwapchain();
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

private:
    vk::Instance m_instance;
    vk::PhysicalDevice m_gpu;
    vk::Device m_device;
#ifndef NDEBUG
    vk::DebugUtilsMessengerEXT m_messenger;
#endif
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapChain;
    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::ImageView> m_swapchainImagesView;
    vk::Format m_swapchainImageFormat = vk::Format::eUndefined;
    vk::Extent2D m_swapchainExtent;
};
} // namespace backend

#endif // LEARNMETALVULKAN_HANDLEVK_H
