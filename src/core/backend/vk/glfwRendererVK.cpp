//
// Created by cwb on 2022/9/5.
//

#include "glfwRendererVK.h"

#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void GLFWRendererVK::initInstance()
{
    // initialize the vk::ApplicationInfo structure
    vk::ApplicationInfo appInfo(s_appName.c_str(), 1, s_engineName.c_str(), 1, VK_API_VERSION_1_1);
    // initialize the vk::InstanceCreateInfo
    auto glfwExtensionCount = 0u;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + glfwExtensionCount);
    glfwExtensionsVector.emplace_back("VK_EXT_debug_utils");
    glfwExtensionsVector.emplace_back("VK_KHR_portability_enumeration");
    auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.setPApplicationInfo(&appInfo);
    instanceCreateInfo.setPEnabledExtensionNames(glfwExtensionsVector);
    instanceCreateInfo.setPEnabledLayerNames(layers);
    instanceCreateInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
    m_instance = vk::createInstanceUnique(instanceCreateInfo);
}

void GLFWRendererVK::initDebugger()
{
    // vk::DispatchLoaderDynamic dldi(*instance);
    auto dldi = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);
    auto messenger = m_instance->createDebugUtilsMessengerEXTUnique(
        vk::DebugUtilsMessengerCreateInfoEXT{ {},
                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
                                              vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                              debugCallback },
        nullptr, dldi);
    (void)messenger;
}

void GLFWRendererVK::initSurface()
{
    VkSurfaceKHR surfaceTmp;
    if (!glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surfaceTmp))
    {
        std::cout << "glfwCreateWindowSurface failed!" << std::endl;
    }
    vk::UniqueSurfaceKHR surface(surfaceTmp, *m_instance);
}

GLFWRendererVK::~GLFWRendererVK()
{
    // 销毁window
    glfwDestroyWindow(m_window);
}

void GLFWRendererVK::initGlfw()
{
    // This tells GLFW to not create an OpenGL context with the window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    initInstance();
    initDebugger();
}

void GLFWRendererVK::swapWindow()
{
}

void GLFWRendererVK::initSwapChain()
{
    initSurface();
}

void GLFWRendererVK::initPhysicalDevice()
{
    /// 挑选一块显卡
}

void GLFWRendererVK::initDevice()
{
}
