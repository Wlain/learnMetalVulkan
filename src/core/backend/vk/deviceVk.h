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
class TextureVK;
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    [[nodiscard]] inline bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
class DeviceVK : public Device
{
public:
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
    const vk::RenderPass& renderPass() const;
    const vk::CommandPool& commandPool() const;
    const std::vector<vk::CommandBuffer>& commandBuffers() const;
    const std::vector<vk::Framebuffer>& swapchainFramebuffers() const;
    const std::vector<vk::Fence>& inflightFences() const;
    const std::vector<vk::Fence>& imagesInflight() const;
    const std::vector<vk::Semaphore>& imageAvailableSemaphores() const;
    const std::vector<vk::Semaphore>& renderFinishedSemaphores() const;
    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
    vk::RenderPassBeginInfo getSingleRenderPassBeginInfo();
    vk::PipelineDepthStencilStateCreateInfo getSingleDepthStencilStateCreateInfo();
    static std::function<void(const vk::CommandBuffer& cb, vk::Pipeline pipeline, vk::PipelineLayout& layout, const vk::DescriptorSet& descriptorSet)> bindPipeline();

private:
    void initInstance();
#ifndef NDEBUG
    void initDebugger();
#endif
    void pickPhysicalDevice();
    void initDevice();
    void initSurface();
    void creatRenderPass();
    void createFrameBuffers();
    void createCommandBuffers();
    void createSyncObjects();
    void createCommandPool();

private:
    void creatSwapChain();
    void createImageViews();
    bool isDeviceSuitable(vk::PhysicalDevice gpu);
    [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice gpu) const;
    void cleanupSwapchain();
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

public:
    constexpr static std::size_t MAX_FRAMES_IN_FLIGHT{ 2 };

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
    std::vector<vk::ImageView> m_swapchainImagesViews;
    std::shared_ptr<TextureVK> m_depthTexture;
    vk::Format m_swapchainImageFormat = vk::Format::eUndefined;
    vk::Extent2D m_swapchainExtent;

    vk::RenderPass m_renderPass;
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    std::vector<vk::Framebuffer> m_swapchainFramebuffers;
    std::vector<vk::Fence> m_inflightFences;
    std::vector<vk::Fence> m_imagesInflight;
    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
};
} // namespace backend

#endif // LEARNMETALVULKAN_HANDLEVK_H
