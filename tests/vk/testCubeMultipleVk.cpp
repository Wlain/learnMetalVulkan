//
// Created by william on 2022/10/21.
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
class TestCubeMultipleVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestCubeMultipleVk() override = default;
    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        buildTextures();
        buildBuffers();
        buildDescriptorsSets();
        buildPipeline();
    }

    void buildTextures()
    {
        m_texture = MAKE_SHARED(m_texture, m_deviceVk);
        m_texture->createWithFileName("textures/test.jpg", true);
    }

    void buildBuffers()
    {
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_deviceVk);
        m_vertexBuffer->create(g_cubeVertex.size() * sizeof(g_cubeVertex[0]), (void*)g_cubeVertex.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_uniformBuffer = MAKE_SHARED(m_uniformBuffer, m_deviceVk);
        m_dynamicAlignment = sizeof(VertMVPMatrixUBO);
        // 字节对齐
        size_t minUboAlignment = m_deviceVk->gpu().getProperties().limits.minUniformBufferOffsetAlignment;
        if (minUboAlignment > 0)
        {
            m_dynamicAlignment = (m_dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }
        size_t bufferSize = g_cubePositions.size() * m_dynamicAlignment;
        g_mvpMatrixUbo.view = glm::translate(g_mvpMatrixUbo.view, glm::vec3(0.0f, 0.0f, -3.0f));
        m_uniformBuffer->create(bufferSize, (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
    }

    void resize(int width, int height) override
    {
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }

    void buildDescriptorsSets()
    {
        m_descriptorSets = MAKE_SHARED(m_descriptorSets, m_deviceVk);
        g_textureShaderResource[0].mode = ShaderResourceMode::Dynamic;
        std::vector descriptorPoolSizes{
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBufferDynamic,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = m_swapchainSize }
        };
        std::map<uint32_t, vk::DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_uniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = m_dynamicAlignment } }
        };
        std::map<uint32_t, vk::DescriptorImageInfo> imageInfos{
            { g_textureBinding, vk::DescriptorImageInfo{
                                    .sampler = m_texture->sampler(),
                                    .imageView = m_texture->imageView(),
                                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal } }
        };
        m_descriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_descriptorSets->createDescriptorSetLayout(g_textureShaderResource);
        m_descriptorSets->createDescriptorSets(bufferInfos, imageInfos);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/texture.vert");
        std::string fragShader = getFileContents("shaders/texture.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVk);
        m_pipeline->setProgram(vertSource, fragShader);
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_descriptorSets->layout(),
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
        m_pipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_pipeline->setTopology(backend::Topology::Triangles);
        m_pipeline->setPipelineLayout(m_pipelineLayout);
        m_pipeline->setViewport();
        m_pipeline->setRasterization();
        m_pipeline->setMultisample();
        m_pipeline->setDepthStencil(m_depthStencilState);
        m_pipeline->setColorBlendAttachment();
        m_pipeline->setRenderPass();
        m_pipeline->build();
        m_render->setPipeline(m_pipeline);
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        auto beginInfo = vk::CommandBufferBeginInfo{};
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            commandBuffers[i].begin(beginInfo);
            static float red{ 1.0f };
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

            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->handle());
            commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
            for (unsigned int j = 0; j < g_cubePositions.size(); j++)
            {
                // calculate the model matrix for each object and pass it to shader before drawing
                uint32_t dynamicOffset = j * static_cast<uint32_t>(m_dynamicAlignment);
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_cubePositions[j]);
                g_mvpMatrixUbo.model = glm::rotate(g_mvpMatrixUbo.model, m_duringTime, glm::vec3(0.5f, 1.0f, 0.0f));
                float angle = 20.0f * j;
                g_mvpMatrixUbo.model = glm::rotate(g_mvpMatrixUbo.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                m_uniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), dynamicOffset);
                commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSets->handle(), dynamicOffset);
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVertex.size()), 1, 0, 0);
            }
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
        m_render->swapBuffers();
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVk{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    std::shared_ptr<BufferVK> m_vertexBuffer;
    std::shared_ptr<BufferVK> m_uniformBuffer;
    std::shared_ptr<TextureVK> m_texture;
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilState;
    vk::PipelineLayout m_pipelineLayout;
    std::shared_ptr<DescriptorSet> m_descriptorSets;
    uint32_t m_swapchainSize{};
    uint32_t m_dynamicAlignment{};
};
} // namespace

void testCubeMultipleVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 800, 640, "Vulkan Example Cube Multiple" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestCubeMultipleVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}