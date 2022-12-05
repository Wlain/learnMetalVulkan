# learnMetal

The Metal/Vulkan version for LearnOpenGL with C++ 17 on MAC-OS.

## dependencies:

* download and install vulkanSDK(1.3.224.1 or latest version)

## 三个图形API的异同点总结以及解决办法
https://alain.xyz/blog/comparison-of-modern-graphics-apis

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

Q7:是否给uniform变量设置默认值

* OpenGL:如果没有设置，会默认设置0
* Metal:如果没有设置，不会默认设置初始化

Q8:是否是一个状态机？

* OpenGL:是一个状态机模式，如果状态没有改变，则状态保持不变
* Metal:不是一个状态机模型，是现代的OOP设计模式的现代图形api

Q9 关于fixup_clipspace修复推导：
```C++
// 已知条件：
glNdc.z = gl_Position.z / gl_Position.w;
mtlNdc.z = metal_Position.z / metal_Position.w;
metal_Position.w = gl_Position.w;
mtlNdc.z = (glNdc.z + 1) * 0.5;

// 推导-》
metal_Position.z = mtlNdc.z / metal_Position.w;
                 = ((glNdc.z + 1) * 0.5) * metal_Position.w;
                 = ((gl_Position.z / gl_Position.w + 1) * 0.5) * gl_Position.w;
                 = (gl_Position.z + gl_Position.w) * 0.5;
```
Q10 metal （UBO）不区分binding 和 location, 如果重了会冲突，比如
```C++
layout (location = 0) in vec4 aPos;

/// 此处 binding = 0, 是不可以的，因为会跟location冲突
layout(binding = 1) uniform VertUniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
```
Q11 关于Vulkan的uniform：只需要知道两个东西
* 一个是binding 的确定, 对应到vk是：WriteDescriptorSet
* 第二个是内存布局的确定，对应到vk是：DescriptorBinding

