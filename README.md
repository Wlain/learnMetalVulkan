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

Q2:OpenGL/Vulkan/Vulkan默认状态值

* OpenGL:
    * 剔除(CullMode)：Back（剔除背面）：https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glCullFace.xhtml
    * 顶点连接顺序(FrontFacingWinding):CCW（逆时针）
* Vulkan:
    * 剔除(CullMode):eNone(不剔除)
    * 顶点连接顺序(FrontFacingWinding):CCW（逆时针）:
* Metal
    * 剔除(CullMode):
    * 顶点连接顺序(FrontFacingWinding):

Q3:坐标系:

* OpenGL:右手坐标系
* Vulkan:左手坐标系
* Metal:左手坐标系

Q4:纹理的坐标系原点

* OpenGL:**左*下角 :因此需要flipY
* Vulkan:**左**上角
* Metal:**左**上角

Q5:viewport坐标系原点

* OpenGL:**左*下角
* Metal:**左**上角
* Vulkan:**左**上角

Q6:NDC坐标，取值范围

* OpenGL:**Y轴*向上（左手坐标系），取值范围[-1,1]
* Metal:**Y轴*向上（左手坐标系），取值范围[0,1]
* Vulkan:**Y轴**向下（右手坐标系），取值范围[0,1]