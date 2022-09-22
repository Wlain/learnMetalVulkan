# learnMetal

The Metal/Vulkan version for LearnOpenGL with C++ 17 on MAC-OS.

## dependencies:

* metal-cpp:clone of Apple Metal CPP project https://developer.apple.com/metal/cpp
* SDL:Download the most recent Development Libraries for macOS from https://www.libsdl.org/download-2.0.php Open the dmg
  and copy the SDL2.framework to /Library/Frameworks
* download and install vulkanSDK(1.3.224.1 or latest version)

## 三个图形API的异同点总结以及解决办法

Q1:原始数据是OpenGL的数据，在Vulkan/metal中解决顶点/坐标的翻转问题?

* Vulkan：将viewport的Y翻转（在vulkan 1.0中添加了一个扩展"VK_KHR_Maintenance1"，该扩展允许viewport的height值为负值。需要注意的是在Vulkan 1.1中该扩展成为了标准）
* metal：翻转FrontFacingWinding即可

Q2:OpenGL默认状态值

* 剔除(CullMode)：Back（剔除背面）：https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glCullFace.xhtml
* 顶点连接顺序(FrontFacingWinding):CCW（逆时针）

Q3:metal/Metal-NDC坐标是:0-1， openGL-NDC是-1-1。

Q4:坐标系:

* OpenGL:右手坐标系
* Vulkan:左手坐标系

Q5:纹理的坐标系原点

* OpenGL:右下角
* Vulkan:左上角
* Metal:左上角