//
// Created by william on 2022/9/4.
//

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <vector>

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void windowGlfwVK()
{
    uint32_t width = 640;
    uint32_t height = 480;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow((int32_t)width, (int32_t)height, "Vulkan GLFW Window", nullptr, nullptr);
    glfwSetKeyCallback(window, key_callback);
    vk::ApplicationInfo appInfo("Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
    auto glfwExtensionCount = 0u;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + glfwExtensionCount);
    glfwExtensionsVector.emplace_back("VK_EXT_debug_utils");
    glfwExtensionsVector.emplace_back("VK_KHR_portability_enumeration");
    auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
    vk::InstanceCreateInfo infos;
    infos.setPApplicationInfo(&appInfo);
    infos.setPEnabledExtensionNames(glfwExtensionsVector);
    infos.setPEnabledLayerNames(layers);
    infos.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
    auto instance = vk::createInstanceUnique(infos);
    // vk::DispatchLoaderDynamic dldi(*instance);
    auto dldi = vk::DispatchLoaderDynamic(*instance, vkGetInstanceProcAddr);
    auto messenger = instance->createDebugUtilsMessengerEXTUnique(
        vk::DebugUtilsMessengerCreateInfoEXT{ {},
                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
                                              vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                              debugCallback },
        nullptr, dldi);
    (void)messenger;
    VkSurfaceKHR surfaceTmp;
    if (!glfwCreateWindowSurface(*instance, window, nullptr, &surfaceTmp))
    {
        std::cout << "glfwCreateWindowSurface failed!" << std::endl;
    }
    vk::UniqueSurfaceKHR surface(surfaceTmp, *instance);

    auto physicalDevices = instance->enumeratePhysicalDevices();
    auto physicalDevice = physicalDevices.front();
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProperties.begin(), std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(), [](vk::QueueFamilyProperties const& qfp) {
                                                        return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                                                    }));
    size_t presentQueueFamilyIndex = 0u;
    for (auto i = 0ul; i < queueFamilyProperties.size(); i++)
    {
        if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface.get()))
        {
            presentQueueFamilyIndex = i;
        }
    }
    std::set<uint32_t> uniqueQueueFamilyIndices = { static_cast<uint32_t>(graphicsQueueFamilyIndex),
                                                    static_cast<uint32_t>(presentQueueFamilyIndex) };
    std::vector<uint32_t> FamilyIndices = { uniqueQueueFamilyIndices.begin(),
                                            uniqueQueueFamilyIndices.end() };
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 0.0f;
    queueCreateInfos.reserve(uniqueQueueFamilyIndices.size());
    for (auto& queueFamilyIndex : uniqueQueueFamilyIndices)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(queueFamilyIndex), 1, &queuePriority);
    }
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::UniqueDevice device = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(),
                                                                                     0u, nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data()));

    uint32_t imageCount = 2;
    struct SM
    {
        vk::SharingMode sharingMode;
        uint32_t familyIndicesCount;
        uint32_t* familyIndicesDataPtr;
    } sharingModeUtil{ (graphicsQueueFamilyIndex != presentQueueFamilyIndex) ?
                           SM{ vk::SharingMode::eConcurrent, 2u, FamilyIndices.data() } :
                           SM{ vk::SharingMode::eExclusive, 0u, static_cast<uint32_t*>(nullptr) } };
    // needed for validation warnings
    auto format = vk::Format::eB8G8R8A8Unorm;
    auto extent = vk::Extent2D{ width, height };
    vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, surface.get(), imageCount, format,
                                                   vk::ColorSpaceKHR::eSrgbNonlinear, extent, 1, vk::ImageUsageFlagBits::eColorAttachment,
                                                   sharingModeUtil.sharingMode, sharingModeUtil.familyIndicesCount,
                                                   sharingModeUtil.familyIndicesDataPtr, vk::SurfaceTransformFlagBitsKHR::eIdentity,
                                                   vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eFifo, true, nullptr);
    auto swapChain = device->createSwapchainKHRUnique(swapChainCreateInfo);
    std::vector<vk::Image> swapChainImages = device->getSwapchainImagesKHR(swapChain.get());
    std::vector<vk::UniqueImageView> imageViews;
    imageViews.reserve(swapChainImages.size());
    for (auto image : swapChainImages)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image,
                                                    vk::ImageViewType::e2D, format,
                                                    vk::ComponentMapping{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                                                                          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA },
                                                    vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        imageViews.push_back(device->createImageViewUnique(imageViewCreateInfo));
    }
    auto colorAttachment = vk::AttachmentDescription{ {}, format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, {}, vk::ImageLayout::ePresentSrcKHR };
    auto colourAttachmentRef = vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };
    auto subpass = vk::SubpassDescription{ {}, vk::PipelineBindPoint::eGraphics,
                                           /*inAttachmentCount*/ 0,
                                           nullptr,
                                           1,
                                           &colourAttachmentRef };
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
    auto imageAvailableSemaphore = device->createSemaphoreUnique(semaphoreCreateInfo);
    auto renderFinishedSemaphore = device->createSemaphoreUnique(semaphoreCreateInfo);
    auto subpassDependency = vk::SubpassDependency{ VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite };
    auto renderPass = device->createRenderPassUnique(vk::RenderPassCreateInfo{ {}, 1, &colorAttachment, 1, &subpass, 1, &subpassDependency });
    auto framebuffers = std::vector<vk::UniqueFramebuffer>(imageCount);
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        framebuffers[i] = device->createFramebufferUnique(vk::FramebufferCreateInfo{
            {}, *renderPass, 1, &(*imageViews[i]), extent.width, extent.height, 1 });
    }
    auto commandPoolUnique = device->createCommandPoolUnique({ {}, static_cast<uint32_t>(graphicsQueueFamilyIndex) });
    auto commandBuffers = device->allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo(commandPoolUnique.get(), vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(framebuffers.size())));
    auto deviceQueue = device->getQueue(static_cast<uint32_t>(graphicsQueueFamilyIndex), 0);
    auto presentQueue = device->getQueue(static_cast<uint32_t>(presentQueueFamilyIndex), 0);
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        auto beginInfo = vk::CommandBufferBeginInfo{};
        commandBuffers[i]->begin(beginInfo);
        vk::ClearValue clearValues{};
        static float red = 1.0f;
        clearValues.color.setFloat32({ red, 0.0f, 0.0f, 1.0f });
        auto renderPassBeginInfo = vk::RenderPassBeginInfo{ renderPass.get(), framebuffers[i].get(), vk::Rect2D{ { 0, 0 }, extent }, 1, &clearValues };
        commandBuffers[i]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        commandBuffers[i]->endRenderPass();
        commandBuffers[i]->end();
    }
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        auto imageIndex = device->acquireNextImageKHR(swapChain.get(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore.get(), {});
        vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        auto submitInfo = vk::SubmitInfo{ 1, &imageAvailableSemaphore.get(), &waitStageMask, 1, &commandBuffers[imageIndex.value].get(), 1, &renderFinishedSemaphore.get() };
        deviceQueue.submit(submitInfo, {});
        auto presentInfo = vk::PresentInfoKHR{ 1, &renderFinishedSemaphore.get(), 1, &swapChain.get(), &imageIndex.value };
        auto result = presentQueue.presentKHR(presentInfo);
        (void)result;
        device->waitIdle();
    }
}
