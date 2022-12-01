//
// Created by william on 2022/9/4.
//

#include "Metal/Metal.hpp"
#include "QuartzCore/QuartzCore.hpp"

#include <SDL.h>

void testWindowSdlMtl()
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SDL Metal window", -1, -1, 640, 480, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    auto swapChain = (CA::MetalLayer*)SDL_RenderGetMetalLayer(renderer);
    auto* gpu = swapChain->device();
    auto* queue = gpu->newCommandQueue();
    MTL::ClearColor color{ 0, 0, 0, 1 };
    bool quit = false;
    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }

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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}