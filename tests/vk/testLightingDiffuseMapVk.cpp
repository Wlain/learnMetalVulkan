//
// Created by william on 2022/12/2.
//
#include "../mesh/globalMeshs.h"
#include "bufferVk.h"
#include "commonHandle.h"
#include "depthStencilStateVk.h"
#include "descriptorSetVk.h"
#include "deviceVk.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "pipelineVk.h"
#include "textureVk.h"
#include "utils.h"

#include <array>
using namespace backend;

namespace
{
class TestDiffuseMapVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestDiffuseMapVk() override = default;

    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVk*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVk*>(m_renderer);
        buildTexture();
        buildBuffers();
        buildDepthStencilStates();
        buildDescriptorsSets();
        buildPipeline();
    }

    void buildTexture()
    {
        m_diffuseMapTexture = MAKE_SHARED(m_diffuseMapTexture, m_render->device());
        m_diffuseMapTexture->createWithFileName("textures/container.jpg", true);
        m_specularMapTexture = MAKE_SHARED(m_specularMapTexture, m_render->device());
        m_specularMapTexture->createWithFileName("textures/container2_specular.png", true);
        m_emissionMapTexture = MAKE_SHARED(m_emissionMapTexture, m_render->device());
        m_emissionMapTexture->createWithFileName("textures/matrix.jpg", true);
    }

    void buildBuffers()
    {
        m_lightCubeVertexBuffer = MAKE_SHARED(m_lightCubeVertexBuffer, m_deviceVk);
        m_lightCubeVertexBuffer->create(g_sphereMesh.size() * sizeof(g_sphereMesh[0]), (void*)g_sphereMesh.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_lightCubeVertUniformBuffer = MAKE_SHARED(m_lightCubeVertUniformBuffer, m_deviceVk);
        m_lightCubeVertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_lightCubeFragUniformBuffer = MAKE_SHARED(m_lightCubeFragUniformBuffer, m_deviceVk);
        m_lightCubeFragUniformBuffer->create(sizeof(g_lightColorUbo), (void*)&g_lightColorUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);

        m_diffuseMapVertexBuffer = MAKE_SHARED(m_diffuseMapVertexBuffer, m_deviceVk);
        m_diffuseMapVertexBuffer->create(g_cubeVerticesWithNormalTexCoord.size() * sizeof(g_cubeVerticesWithNormalTexCoord[0]), (void*)g_cubeVerticesWithNormalTexCoord.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_diffuseMapVertUniformBuffer = MAKE_SHARED(m_diffuseMapVertUniformBuffer, m_deviceVk);
        m_diffuseMapVertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_diffuseMapFragUniformBuffer = MAKE_SHARED(m_diffuseMapFragUniformBuffer, m_deviceVk);
        m_diffuseMapFragUniformBuffer->create(sizeof(g_fragDiffuseMapUBO), (void*)&g_fragDiffuseMapUBO, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void buildDepthStencilStates()
    {
        m_depthStencilState = MAKE_SHARED(m_depthStencilState, m_deviceVk);
        m_depthStencilState->setDepthCompareOp(CompareOp::Less);
        m_depthStencilState->setDepthTestEnable(true);
        m_depthStencilState->setDepthWriteEnable(true);
    }

    void buildDescriptorsSets()
    {
        m_lightCubeDescriptorSets = MAKE_SHARED(m_lightCubeDescriptorSets, m_deviceVk);
        std::vector descriptorPoolSizes{
            vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer,
                                    .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize }

        };
        std::map<uint32_t, vk::DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_lightCubeVertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_lightSphereUboBinding, vk::DescriptorBufferInfo{ .buffer = m_lightCubeFragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragLightSphereUBO) } },
        };
        m_lightCubeDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_lightCubeDescriptorSets->createDescriptorSetLayout(g_lightSphereShaderResource);
        m_lightCubeDescriptorSets->createDescriptorSets(bufferInfos, {});

        m_diffuseMapDescriptorSets = MAKE_SHARED(m_diffuseMapDescriptorSets, m_deviceVk);
        descriptorPoolSizes = {
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = m_swapchainSize }
        };
        bufferInfos = {
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_diffuseMapVertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_fragDiffuseMapUboBinding, vk::DescriptorBufferInfo{ .buffer = m_diffuseMapFragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragDiffuseMapUBO) } },
        };
        std::map<uint32_t, vk::DescriptorImageInfo> imageInfos{
            { g_diffuseTextureBinding, vk::DescriptorImageInfo{
                                           .sampler = m_diffuseMapTexture->sampler(),
                                           .imageView = m_diffuseMapTexture->imageView(),
                                           .imageLayout = m_diffuseMapTexture->imageLayout() } },
            { g_specularTextureBinding, vk::DescriptorImageInfo{ .sampler = m_specularMapTexture->sampler(), .imageView = m_specularMapTexture->imageView(), .imageLayout = m_specularMapTexture->imageLayout() } },
            { g_emissionTextureBinding, vk::DescriptorImageInfo{ .sampler = m_emissionMapTexture->sampler(), .imageView = m_emissionMapTexture->imageView(), .imageLayout = m_emissionMapTexture->imageLayout() } }
        };
        m_diffuseMapDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_diffuseMapDescriptorSets->createDescriptorSetLayout(g_diffuseMapShaderResource);
        m_diffuseMapDescriptorSets->createDescriptorSets(bufferInfos, imageInfos);
    }

    void buildPipeline()
    {
        // lightCube
        std::string vertSource = getFileContents("shaders/lightColorCube.vert");
        std::string fragShader = getFileContents("shaders/lightColorCube.frag");
        m_lightCubePipeline = MAKE_SHARED(m_lightCubePipeline, m_deviceVk);
        m_lightCubePipeline->setProgram(vertSource, fragShader);
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_lightCubeDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_lightCubePipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_lightCubePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_lightCubePipeline->setTopology(backend::Topology::Triangles);
        m_lightCubePipeline->setPipelineLayout(m_lightCubePipelineLayout);
        m_lightCubePipeline->setViewport();
        m_lightCubePipeline->setRasterization();
        m_lightCubePipeline->setMultisample();
        m_lightCubePipeline->setDepthStencil(m_depthStencilState->handle());
        m_lightCubePipeline->setColorBlendAttachment();
        m_lightCubePipeline->setRenderPass();
        m_lightCubePipeline->build();

        // basicLighting
        vertSource = getFileContents("shaders/lightingDiffuseMap.vert");
        fragShader = getFileContents("shaders/lightingDiffuseMap.frag");
        m_diffuseMapPipeline = MAKE_SHARED(m_diffuseMapPipeline, m_render->device());
        m_diffuseMapPipeline->setProgram(vertSource, fragShader);
        pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_diffuseMapDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_diffuseMapPipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_diffuseMapPipeline->setAttributeDescription(getThreeElemsAttributesDescriptions());
        m_diffuseMapPipeline->setTopology(backend::Topology::Triangles);
        m_diffuseMapPipeline->setPipelineLayout(m_diffuseMapPipelineLayout);
        m_diffuseMapPipeline->setViewport();
        m_diffuseMapPipeline->setRasterization();
        m_diffuseMapPipeline->setMultisample();
        m_diffuseMapPipeline->setDepthStencil(m_depthStencilState->handle());
        m_diffuseMapPipeline->setColorBlendAttachment();
        m_diffuseMapPipeline->setRenderPass();
        m_diffuseMapPipeline->build();
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        auto beginInfo = vk::CommandBufferBeginInfo{};
        auto renderPassInfo = m_deviceVk->getSingleRenderPassBeginInfo();
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            renderPassInfo.setFramebuffer(framebuffer[i]);
            commandBuffers[i].begin(beginInfo);
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            // draw lighting sphere
            {
                commandBuffers[i].bindVertexBuffers(0, { m_lightCubeVertexBuffer->buffer() }, { 0 });
                // calculate the model matrix for each object and pass it to shader before drawing
                g_fragDiffuseMapUBO.light.position.x = 1.0f + sin(m_duringTime) * 2.0f;
                g_fragDiffuseMapUBO.light.position.y = sin(m_duringTime / 2.0f) * 1.0f;
                // calculate the model matrix for each object and pass it to shader before drawing
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_fragDiffuseMapUBO.light.position));
                g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.05f)); // a smaller cube
                m_lightCubeVertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), 0);
                g_lightColorUbo.lightColor.x = static_cast<float>(std::abs(sin(m_duringTime * 2.0)));
                g_lightColorUbo.lightColor.y = static_cast<float>(std::abs(sin(m_duringTime * 0.7)));
                g_lightColorUbo.lightColor.z = static_cast<float>(std::abs(sin(m_duringTime * 1.3)));
                m_lightCubeFragUniformBuffer->update(&g_lightColorUbo, sizeof(g_lightColorUbo), 0);
                DeviceVk::bindPipeline()(commandBuffers[i], m_lightCubePipeline->handle(), m_lightCubePipelineLayout, m_lightCubeDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_sphereMesh.size()), 1, 0, 0);
            }
            // draw diffuseMap cube
            {
                commandBuffers[i].bindVertexBuffers(0, { m_diffuseMapVertexBuffer->buffer() }, { 0 });
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                m_diffuseMapVertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), 0);
                g_fragDiffuseMapUBO.viewPos = glm::vec4(m_camera.position, 1.0f);
                g_fragDiffuseMapUBO.light.diffuse = g_lightColorUbo.lightColor * glm::vec4(0.5f);        // decrease the influence;
                g_fragDiffuseMapUBO.light.ambient = g_fragDiffuseMapUBO.light.diffuse * glm::vec4(0.2f); // decrease the influence
                g_fragDiffuseMapUBO.light.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
                m_diffuseMapFragUniformBuffer->update(&g_fragDiffuseMapUBO, sizeof(g_fragDiffuseMapUBO), 0);
                DeviceVk::bindPipeline()(commandBuffers[i], m_diffuseMapPipeline->handle(), m_diffuseMapPipelineLayout, m_diffuseMapDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVerticesWithNormalTexCoord.size()), 1, 0, 0);
            }
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVk* m_render{ nullptr };
    DeviceVk* m_deviceVk{ nullptr };

    std::shared_ptr<DepthStencilStateVk> m_depthStencilState;

    std::shared_ptr<PipelineVk> m_lightCubePipeline;
    std::shared_ptr<BufferVk> m_lightCubeVertexBuffer;
    std::shared_ptr<BufferVk> m_lightCubeVertUniformBuffer;
    std::shared_ptr<BufferVk> m_lightCubeFragUniformBuffer;
    vk::PipelineLayout m_lightCubePipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_lightCubeDescriptorSets;

    std::shared_ptr<PipelineVk> m_diffuseMapPipeline;
    std::shared_ptr<BufferVk> m_diffuseMapVertexBuffer;
    std::shared_ptr<BufferVk> m_diffuseMapVertUniformBuffer;
    std::shared_ptr<BufferVk> m_diffuseMapFragUniformBuffer;
    std::shared_ptr<TextureVk> m_diffuseMapTexture;
    std::shared_ptr<TextureVk> m_specularMapTexture;
    std::shared_ptr<TextureVk> m_emissionMapTexture;
    vk::PipelineLayout m_diffuseMapPipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_diffuseMapDescriptorSets;
    uint32_t m_swapchainSize{};
};
} // namespace

void testLightingDiffuseMapVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 800, 640, "Vulkan Example diffuseMap" };
    DeviceVk handle(info);
    handle.init();
    GLFWRendererVk renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestDiffuseMapVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}