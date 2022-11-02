//
// Created by william on 2022/9/23.
//
#include "../mesh/globalMeshs.h"
#include "bufferVk.h"
#include "commonHandle.h"
#include "deviceVk.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "pipelineVk.h"
#include "textureVk.h"
#include "utils.h"

using namespace backend;

namespace
{
class TestTextureVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestTextureVk() override
    {
        auto device = m_deviceVk->handle();
        device.destroy(m_descriptorSetLayout);
        device.freeDescriptorSets(m_descriptorPool, m_descriptorSets);
        device.destroy(m_descriptorPool);
    }
    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        m_texture = MAKE_SHARED(m_texture, m_deviceVk);
        m_texture->createWithFileName("textures/test.jpg", true);
        buildBuffers();
        buildPipeline();
    }

    void buildBuffers()
    {
        m_vertexBuffer = MAKE_SHARED(m_vertexBuffer, m_deviceVk);
        m_vertexBuffer->create(g_quadVertex.size() * sizeof(g_quadVertex[0]), (void*)g_quadVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_indexBuffer = MAKE_SHARED(m_indexBuffer, m_deviceVk);
        m_indexBuffer->create(g_quadIndices.size() * sizeof(g_quadIndices[0]), (void*)g_quadIndices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::IndexBuffer);
        m_uniformBuffer = MAKE_SHARED(m_uniformBuffer, m_deviceVk);
        m_uniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
    }

    vk::DescriptorSetLayout& createDescriptorSetLayout()
    {
        if (!m_descriptorSetLayout)
        {
            auto uboLayoutBinding = vk::DescriptorSetLayoutBinding{
                .binding = g_mvpMatrixUboBinding,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .pImmutableSamplers = nullptr // optional (only relevant to Image Sampling;
            };
            auto samplerLayoutBinding = vk::DescriptorSetLayoutBinding{
                .binding = g_textureBinding,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
                .pImmutableSamplers = nullptr,
            };
            std::array binding = { uboLayoutBinding, samplerLayoutBinding };
            auto layoutInfo = vk::DescriptorSetLayoutCreateInfo{
                .bindingCount = binding.size(),
                .pBindings = binding.data()
            };
            m_descriptorSetLayout = m_deviceVk->handle().createDescriptorSetLayout(layoutInfo);
        }
        return m_descriptorSetLayout;
    }

    vk::DescriptorPool& createDescriptorPool()
    {
        if (!m_descriptorPool)
        {
            std::array<vk::DescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
            poolSizes[0].descriptorCount = m_swapchainSize;
            poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
            poolSizes[1].descriptorCount = m_swapchainSize;
            auto poolInfo = vk::DescriptorPoolCreateInfo{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = m_swapchainSize,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data()
            };
            m_descriptorPool = m_deviceVk->handle().createDescriptorPool(poolInfo);
        }
        return m_descriptorPool;
    }

    std::vector<vk::DescriptorSet>& createDescriptorSets()
    {
        if (m_descriptorSets.empty())
        {
            std::vector<vk::DescriptorSetLayout> layouts(m_swapchainSize, createDescriptorSetLayout());

            auto allocInfo = vk::DescriptorSetAllocateInfo{
                .descriptorPool = createDescriptorPool(),
                .descriptorSetCount = m_swapchainSize,
                .pSetLayouts = layouts.data()
            };
            m_descriptorSets.resize(m_swapchainSize);
            m_descriptorSets = m_deviceVk->handle().allocateDescriptorSets(allocInfo);
            for (size_t i = 0; i < m_swapchainSize; i++)
            {
                auto bufferInfo = vk::DescriptorBufferInfo{
                    .buffer = m_uniformBuffer->buffer(),
                    .offset = 0,
                    .range = sizeof(VertMVPMatrixUBO)
                };
                auto imageInfo = vk::DescriptorImageInfo{
                    .sampler = m_texture->sampler(),
                    .imageView = m_texture->imageView(),
                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                };
                std::array descriptorWrites = {
                    vk::WriteDescriptorSet{
                        .dstSet = createDescriptorSets()[i],
                        .dstBinding = g_mvpMatrixUboBinding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                        .pBufferInfo = &bufferInfo },
                    vk::WriteDescriptorSet{
                        .dstSet = createDescriptorSets()[i],
                        .dstBinding = g_textureBinding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &imageInfo }
                };
                m_deviceVk->handle().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
        return m_descriptorSets;
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/texture.vert");
        std::string fragShader = getFileContents("shaders/texture.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVk);
        m_pipeline->setProgram(vertSource, fragShader);
        m_bindingDescription = getPosTexCoordBindingDescription();
        m_attributeDescriptions = getPosTexCoordAttributeDescriptions();
        m_vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &m_bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size()),
            .pVertexAttributeDescriptions = m_attributeDescriptions.data()
        };
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &createDescriptorSetLayout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_pipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_pipeline->initVertexBuffer(m_vertexInputInfo);
        m_pipeline->setTopology(backend::Topology::Triangles);
        m_pipeline->setPipelineLayout(m_pipelineLayout);
        m_pipeline->setViewport();
        m_pipeline->setRasterization();
        m_pipeline->setMultisample();
        //        m_pipeline->setDepthStencil();
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
            static float red{ 0.0f };
            red = red > 1.0f ? 0.0 : red + 0.001f;
            std::array<vk::ClearValue, 2> clearValues = {
                vk::ClearValue{
                    .color = { .float32 = std::array<float, 4>{ red, 0.0f, 0.0f, 1.0f } } },
                vk::ClearValue{
                    .depthStencil = { 1.0f, 0 } }
            };
            auto renderPassInfo = vk::RenderPassBeginInfo{
                .renderPass = m_deviceVk->renderPass(),
                .framebuffer = framebuffer[i],
                .renderArea = {
                    .offset = { 0, 0 },
                    .extent = m_deviceVk->swapchainExtent() },
                .clearValueCount = 2,
                .pClearValues = clearValues.data()
            };
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->handle());
            commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
            commandBuffers[i].bindIndexBuffer(m_indexBuffer->buffer(), 0, vk::IndexType::eUint16);
            commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &createDescriptorSets()[i], 0, nullptr);
            commandBuffers[i].drawIndexed(static_cast<std::uint32_t>(g_quadIndices.size()), 1, 0, 0, 0);
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVk{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    std::shared_ptr<BufferVK> m_vertexBuffer;
    std::shared_ptr<BufferVK> m_indexBuffer;
    std::shared_ptr<BufferVK> m_uniformBuffer;
    std::shared_ptr<TextureVK> m_texture;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    std::array<vk::VertexInputAttributeDescription, 2> m_attributeDescriptions;
    vk::PipelineLayout m_pipelineLayout;
    vk::VertexInputBindingDescription m_bindingDescription;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::DescriptorPool m_descriptorPool;
    std::vector<vk::DescriptorSet> m_descriptorSets;
    std::array<vk::VertexInputAttributeDescription, 2> m_vertexInputAttribute;
    uint32_t m_swapchainSize{};
};
} // namespace

void testTextureVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 640, "Vulkan Texture Use EBO" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestTextureVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}