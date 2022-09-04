//
// Created by william on 2022/9/4.
//

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

extern void* createLayer(GLFWwindow* window, double width, double height, void* device);

void windowGlfwMtl()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    constexpr int width = 640;
    constexpr int height = 480;
    auto* window = glfwCreateWindow(width, height, "Metal Example window", nullptr, nullptr);
    auto* gpu = MTL::CreateSystemDefaultDevice();
    CA::MetalLayer* swapChain = (CA::MetalLayer*)createLayer(window, width, height, gpu);
    auto* queue = gpu->newCommandQueue();
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    MTL::ClearColor color{ 0, 0, 0, 1 };
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        auto* surface = swapChain->nextDrawable();
        color.red = (color.red > 1.0) ? 0 : color.red + 0.01;
        MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::renderPassDescriptor();
        pass->colorAttachments()->object(0)->setClearColor(color);
        pass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        pass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
        pass->colorAttachments()->object(0)->setTexture(surface->texture());

        auto* buffer = queue->commandBuffer();
        auto* encoder = buffer->renderCommandEncoder(pass);
        encoder->endEncoding();
        buffer->presentDrawable(surface);
        buffer->commit();
    }
}