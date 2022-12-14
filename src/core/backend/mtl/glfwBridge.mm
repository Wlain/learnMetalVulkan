#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3native.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
namespace backend
{
void* createLayer(GLFWwindow* window, double width, double height, void* device)
{
    CGSize size = {};
    size.height = height;
    size.width = width;

    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = (__bridge id<MTLDevice>)device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.drawableSize = size;

    NSWindow* nsWindow = (NSWindow*)glfwGetCocoaWindow(window);
    nsWindow.contentView.layer = layer;
    nsWindow.contentView.wantsLayer = YES;
    return layer;
}

void* nextDrawable(void* layer)
{
    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    return [metalLayer nextDrawable];
}
}