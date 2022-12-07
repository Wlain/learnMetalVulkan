//
// Created by william on 2022/9/23.
//
#include "../mesh/globalMeshs.h"
#include "bufferVk.h"
#include "commonHandle.h"
#include "descriptorSetVk.h"
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
    ~TestTextureVk() override = default;
    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        buildTextures();
        buildBuffers();
        buildDescriptorsSet();
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
        m_vertexBuffer->create(g_quadVertex.size() * sizeof(g_quadVertex[0]), (void*)g_quadVertex.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_indexBuffer = MAKE_SHARED(m_indexBuffer, m_deviceVk);
        m_indexBuffer->create(g_quadIndices.size() * sizeof(g_quadIndices[0]), (void*)g_quadIndices.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::IndexBuffer);
        m_uniformBuffer = MAKE_SHARED(m_uniformBuffer, m_deviceVk);
        m_uniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
    }

    void buildDescriptorsSet()
    {
        m_descriptorSet = MAKE_SHARED(m_descriptorSet, m_deviceVk);
        std::vector descriptorPoolSizes{
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = m_swapchainSize }
        };
        std::map<uint32_t, vk::DescriptorBufferInfo> bufferInfos{
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_uniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        std::map<uint32_t, vk::DescriptorImageInfo> imageInfos{
            { g_textureBinding, vk::DescriptorImageInfo{
                                    .sampler = m_texture->sampler(),
                                    .imageView = m_texture->imageView(),
                                    .imageLayout = m_texture->imageLayout() } }
        };
        m_descriptorSet->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_descriptorSet->createDescriptorSetLayout(g_textureShaderResource);
        m_descriptorSet->createDescriptorSets(bufferInfos, imageInfos);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/texture.vert");
        std::string fragShader = getFileContents("shaders/texture.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVk);
        m_pipeline->setProgram(vertSource, fragShader);
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_descriptorSet->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_pipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_pipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_pipeline->setTopology(backend::Topology::Triangles);
        m_pipeline->setPipelineLayout(m_pipelineLayout);
        m_pipeline->setViewport();
        m_pipeline->setRasterization();
        m_pipeline->setMultisample();
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
        auto renderPassInfo = m_deviceVk->getSingleRenderPassBeginInfo();
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            renderPassInfo.setFramebuffer(framebuffer[i]);
            commandBuffers[i].begin(beginInfo);
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            m_deviceVk->bindPipeline()(commandBuffers[i], m_pipeline->handle(), m_pipelineLayout, m_descriptorSet->handle());
            commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
            commandBuffers[i].bindIndexBuffer(m_indexBuffer->buffer(), 0, vk::IndexType::eUint16);
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
    vk::PipelineLayout m_pipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_descriptorSet;
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