//
// Created by william on 2022/9/3.
//
extern void testDemoTriangleVk();
extern void testDemoTextureVk();
extern void testWindowSdlVk();
extern void testWindowSdlMtl();

extern void testWindowGl();
extern void testWindowVk();
extern void testWindowMtl();

extern void testTriangleGl();
extern void testTriangleVk();
extern void testTriangleMtl();

extern void testQuadEboGl();
extern void testQuadEboVk();
extern void testQuadEboMtl();

extern void testTextureGl();
extern void testTextureVk();
extern void testTextureMtl();

extern void testCubeGl();
extern void testCubeVk();
extern void testCubeMtl();

extern void testCubeMultipleGl();
extern void testCubeMultipleVk();
extern void testCubeMultipleMtl();

extern void testCameraGl();
extern void testCameraVk();
extern void testCameraMtl();


int main()
{
    testCubeMultipleVk();
    return 0;
}
