//
// Created by william on 2022/9/3.
//
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

extern void testLightingColorsGl();
extern void testLightingColorsVk();
extern void testLightingColorsMtl();

extern void testBasicLightingGl();
extern void testBasicLightingVk();
extern void testBasicLightingMtl();

extern void testMaterialsGl();
extern void testMaterialsVk();
extern void testMaterialsMtl();

extern void testLightingDiffuseMapGl();
extern void testLightingDiffuseMapVk();
extern void testLightingDiffuseMapMtl();

int main()
{
    testLightingDiffuseMapGl();
    return 0;
}