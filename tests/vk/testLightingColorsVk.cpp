//
// Created by william on 2022/10/31.
//

#include "../mesh/globalMeshs.h"
#include "bufferVk.h"
#include "commonHandle.h"
#include "descriptorSet.h"
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
class TestLightingColorsVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestLightingColorsVk() override = default;

    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        buildBuffers();
        buildDescriptorsSets();
        buildPipeline();
    }

    void buildBuffers()
    {
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_deviceVk);
        m_vertexBuffer->create(g_cubeVertices.size() * sizeof(g_cubeVertices[0]), (void*)g_cubeVertices.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_vertUniformBuffer = MAKE_SHARED(m_vertUniformBuffer, m_deviceVk);
        m_fragUniformBuffer = MAKE_SHARED(m_fragUniformBuffer, m_deviceVk);
        m_vertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_fragUniformBuffer->create(sizeof(g_lightingColorUbo), (void*)&g_lightingColorUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
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
                                         .buffer = m_vertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        m_lightCubeDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_lightCubeDescriptorSets->createDescriptorSetLayout(g_lightCubeShaderResource);
        m_lightCubeDescriptorSets->createDescriptorSets(bufferInfos, {});

        m_colorsDescriptorSets = MAKE_SHARED(m_colorsDescriptorSets, m_deviceVk);
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
                                         .buffer = m_vertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_lightingColorUboBinding, vk::DescriptorBufferInfo{ .buffer = m_fragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragLightingColorUBO) } },
        };
        m_colorsDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_colorsDescriptorSets->createDescriptorSetLayout(g_colorsShaderResource);
        m_colorsDescriptorSets->createDescriptorSets(bufferInfos, {});
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
        m_depthStencilState = vk::PipelineDepthStencilStateCreateInfo{
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthCompareOp = vk::CompareOp::eLess,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
            .front.failOp = vk::StencilOp::eKeep,
            .front.passOp = vk::StencilOp::eKeep,
            .front.compareOp = vk::CompareOp::eAlways,
            .back.failOp = vk::StencilOp::eKeep,
            .back.passOp = vk::StencilOp::eKeep,
            .back.compareOp = vk::CompareOp::eAlways
        };
        m_lightCubePipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_lightCubePipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_lightCubePipeline->setTopology(backend::Topology::Triangles);
        m_lightCubePipeline->setPipelineLayout(m_lightCubePipelineLayout);
        m_lightCubePipeline->setViewport();
        m_lightCubePipeline->setRasterization();
        m_lightCubePipeline->setMultisample();
        m_lightCubePipeline->setDepthStencil(m_depthStencilState);
        m_lightCubePipeline->setColorBlendAttachment();
        m_lightCubePipeline->setRenderPass();
        m_lightCubePipeline->build();

        // color
        vertSource = getFileContents("shaders/colors.vert");
        fragShader = getFileContents("shaders/colors.frag");
        m_colorsPipeline = MAKE_SHARED(m_colorsPipeline, m_render->device());
        m_colorsPipeline->setProgram(vertSource, fragShader);
        pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_colorsDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_colorsPipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_colorsPipeline->setAttributeDescription(getOneElemAttributesDescriptions());
        m_colorsPipeline->setTopology(backend::Topology::Triangles);
        m_colorsPipeline->setPipelineLayout(m_colorsPipelineLayout);
        m_colorsPipeline->setViewport();
        m_colorsPipeline->setRasterization();
        m_colorsPipeline->setMultisample();
        m_colorsPipeline->setDepthStencil(m_depthStencilState);
        m_colorsPipeline->setColorBlendAttachment();
        m_colorsPipeline->setRenderPass();
        m_colorsPipeline->build();
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        auto bind = [](const vk::CommandBuffer& cb, vk::Pipeline pipeline, vk::PipelineLayout& layout, const vk::DescriptorSet& descriptorSet) {
            cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, descriptorSet, nullptr);
            cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        };
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            auto beginInfo = vk::CommandBufferBeginInfo{};
            commandBuffers[i].begin(beginInfo);
            std::array clearValues = {
                vk::ClearValue{ .color = { .float32 = std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 1.0f } } },
                vk::ClearValue{ .depthStencil = { 1.0f, 0 } }
            };
            auto renderPassInfo = vk::RenderPassBeginInfo{
                .renderPass = m_deviceVk->renderPass(),
                .framebuffer = framebuffer[i],
                .renderArea = {
                    .offset = { 0, 0 },
                    .extent = m_deviceVk->swapchainExtent() },
                .clearValueCount = 2,
                .pClearValues = clearValues.data(),
            };
            {
                commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
                commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
                // calculate the model matrix for each object and pass it to shader before drawing
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_lightPos);
                g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
                m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
                bind(commandBuffers[i], m_lightCubePipeline->handle(), m_lightCubePipelineLayout, m_lightCubeDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVertices.size()), 1, 0, 0);
                // calculate the model matrix for each object and pass it to shader before drawing
                commandBuffers[i].endRenderPass();
            }
            {
                commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
                commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
                bind(commandBuffers[i], m_colorsPipeline->handle(), m_colorsPipelineLayout, m_colorsDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVertices.size()), 1, 0, 0);
                commandBuffers[i].endRenderPass();
            }
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVk{ nullptr };
    std::shared_ptr<PipelineVk> m_lightCubePipeline;
    std::shared_ptr<PipelineVk> m_colorsPipeline;
    std::shared_ptr<BufferVK> m_vertexBuffer;
    std::shared_ptr<BufferVK> m_vertUniformBuffer;
    std::shared_ptr<BufferVK> m_fragUniformBuffer;
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilState;
    vk::PipelineLayout m_lightCubePipelineLayout;
    vk::PipelineLayout m_colorsPipelineLayout;
    std::shared_ptr<DescriptorSet> m_colorsDescriptorSets;
    std::shared_ptr<DescriptorSet> m_lightCubeDescriptorSets;
    uint32_t m_swapchainSize{};
};
} // namespace

void testLightingColorsVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 800, 640, "Vulkan Example Lighting Colors" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestLightingColorsVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}