//
// Created by william on 2022/11/27.
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
class TestBasicLightingVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestBasicLightingVk() override = default;

    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        buildBuffers();
        buildDepthStencilStates();
        buildDescriptorsSets();
        buildPipeline();
    }

    void buildBuffers()
    {
        m_lightCubeVertexBuffer = MAKE_SHARED(m_lightCubeVertexBuffer, m_deviceVk);
        m_lightCubeVertexBuffer->create(g_cubeVertices.size() * sizeof(g_cubeVertices[0]), (void*)g_cubeVertices.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_lightCubeVertUniformBuffer = MAKE_SHARED(m_lightCubeVertUniformBuffer, m_deviceVk);
        m_lightCubeVertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);

        m_basicLightingVertexBuffer = MAKE_SHARED(m_basicLightingVertexBuffer, m_deviceVk);
        m_basicLightingVertexBuffer->create(g_cubeVerticesWithNormal.size() * sizeof(g_cubeVerticesWithNormal[0]), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_basicLightingVertUniformBuffer = MAKE_SHARED(m_basicLightingVertUniformBuffer, m_deviceVk);
        m_basicLightingVertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_basicLightingFragUniformBuffer = MAKE_SHARED(m_basicLightingFragUniformBuffer, m_deviceVk);
        m_basicLightingFragUniformBuffer->create(sizeof(g_basicLightingColorUbo), (void*)&g_basicLightingColorUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
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
        };
        std::map<uint32_t, vk::DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_lightCubeVertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        m_lightCubeDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_lightCubeDescriptorSets->createDescriptorSetLayout(g_lightCubeShaderResource);
        m_lightCubeDescriptorSets->createDescriptorSets(bufferInfos, {});

        m_basicLightingDescriptorSets = MAKE_SHARED(m_basicLightingDescriptorSets, m_deviceVk);
        descriptorPoolSizes = {
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize },
        };
        bufferInfos = {
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_basicLightingVertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_lightingColorUboBinding, vk::DescriptorBufferInfo{ .buffer = m_basicLightingFragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragBasicLightingColorUBO) } },
        };
        m_basicLightingDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_basicLightingDescriptorSets->createDescriptorSetLayout(g_basicLightingShaderResource);
        m_basicLightingDescriptorSets->createDescriptorSets(bufferInfos, {});
    }

    void buildPipeline()
    {
        // lightCube
        std::string vertSource = getFileContents("shaders/lightCube.vert");
        std::string fragShader = getFileContents("shaders/lightCube.frag");
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
        vertSource = getFileContents("shaders/basicLighting.vert");
        fragShader = getFileContents("shaders/basicLighting.frag");
        m_basicLightingPipeline = MAKE_SHARED(m_basicLightingPipeline, m_render->device());
        m_basicLightingPipeline->setProgram(vertSource, fragShader);
        pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_basicLightingDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_basicLightingPipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_basicLightingPipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_basicLightingPipeline->setTopology(backend::Topology::Triangles);
        m_basicLightingPipeline->setPipelineLayout(m_basicLightingPipelineLayout);
        m_basicLightingPipeline->setViewport();
        m_basicLightingPipeline->setRasterization();
        m_basicLightingPipeline->setMultisample();
        m_basicLightingPipeline->setDepthStencil(m_depthStencilState->handle());
        m_basicLightingPipeline->setColorBlendAttachment();
        m_basicLightingPipeline->setRenderPass();
        m_basicLightingPipeline->build();
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
            // draw lighting cube
            {
                commandBuffers[i].bindVertexBuffers(0, { m_lightCubeVertexBuffer->buffer() }, { 0 });
                // calculate the model matrix for each object and pass it to shader before drawing
                g_basicLightingColorUbo.lightPos.x = 1.0f + sin(m_duringTime) * 2.0f;
                g_basicLightingColorUbo.lightPos.y = sin(m_duringTime / 2.0f) * 1.0f;
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_basicLightingColorUbo.lightPos));
                g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
                m_lightCubeVertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
                DeviceVK::bindPipeline()(commandBuffers[i], m_lightCubePipeline->handle(), m_lightCubePipelineLayout, m_lightCubeDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVertices.size()), 1, 0, 0);
            }
            // draw basic lighting cube
            {
                commandBuffers[i].bindVertexBuffers(0, { m_basicLightingVertexBuffer->buffer() }, { 0 });
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                m_basicLightingVertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
                g_basicLightingColorUbo.viewPos = glm::vec4(m_camera.position, 1.0f);
                m_basicLightingFragUniformBuffer->update(&g_basicLightingColorUbo, sizeof(FragBasicLightingColorUBO), 0);
                DeviceVK::bindPipeline()(commandBuffers[i], m_basicLightingPipeline->handle(), m_basicLightingPipelineLayout, m_basicLightingDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVerticesWithNormal.size()), 1, 0, 0);
            }
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVk{ nullptr };

    std::shared_ptr<DepthStencilStateVk> m_depthStencilState;
    std::shared_ptr<PipelineVk> m_lightCubePipeline;
    std::shared_ptr<BufferVK> m_lightCubeVertexBuffer;
    std::shared_ptr<BufferVK> m_lightCubeVertUniformBuffer;
    vk::PipelineLayout m_lightCubePipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_lightCubeDescriptorSets;

    std::shared_ptr<PipelineVk> m_basicLightingPipeline;
    std::shared_ptr<BufferVK> m_basicLightingVertexBuffer;
    std::shared_ptr<BufferVK> m_basicLightingVertUniformBuffer;
    std::shared_ptr<BufferVK> m_basicLightingFragUniformBuffer;
    vk::PipelineLayout m_basicLightingPipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_basicLightingDescriptorSets;
    uint32_t m_swapchainSize{};
};
} // namespace

void testBasicLightingVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 800, 640, "Vulkan Example Basic Lighting" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestBasicLightingVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}