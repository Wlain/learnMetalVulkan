//
// Created by william on 2022/12/8.
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
class TestDepthViewVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestDepthViewVk() override = default;
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
        m_cubeTexture = MAKE_SHARED(m_cubeTexture, m_render->device());
        m_cubeTexture->createWithFileName("textures/marble.jpg", true);
        m_planeTexture = MAKE_SHARED(m_cubeTexture, m_render->device());
        m_planeTexture->createWithFileName("textures/metal.png", true);
    }

    void buildBuffers()
    {
        m_cubeVertexBuffer = MAKE_SHARED(m_cubeVertexBuffer, m_deviceVk);
        m_cubeVertexBuffer->create(g_cubeVerticesPosTexCoord.size() * sizeof(g_cubeVerticesPosTexCoord[0]), (void*)g_cubeVerticesPosTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_cubeUniformBuffer = MAKE_SHARED(m_cubeUniformBuffer, m_deviceVk);
        m_dynamicAlignment = sizeof(VertMVPMatrixUBO);
        // 字节对齐
        size_t minUboAlignment = m_deviceVk->gpu().getProperties().limits.minUniformBufferOffsetAlignment;
        if (minUboAlignment > 0)
        {
            m_dynamicAlignment = (m_dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }
        size_t bufferSize = g_cubePositions.size() * m_dynamicAlignment;
        m_cubeUniformBuffer->create(bufferSize, (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);

        m_planeVertexBuffer = MAKE_SHARED(m_planeVertexBuffer, m_deviceVk);
        m_planeVertexBuffer->create(g_cubeVerticesPosTexCoord.size() * sizeof(g_planeVerticesPosTexCoord[0]), (void*)g_planeVerticesPosTexCoord.data(), Buffer::BufferUsage::StaticDraw, Buffer::BufferType::VertexBuffer);
        m_planeUniformBuffer = MAKE_SHARED(m_planeUniformBuffer, m_deviceVk);
        m_planeUniformBuffer->create(sizeof(VertMVPMatrixUBO), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::StaticDraw, Buffer::BufferType::UniformBuffer);
    }

    void buildDepthStencilStates()
    {
        m_depthStencilState = MAKE_SHARED(m_depthStencilState, m_render->device());
        m_depthStencilState->setDepthCompareOp(CompareOp::Less);
        m_depthStencilState->setDepthTestEnable(true);
        m_depthStencilState->setDepthWriteEnable(true);
    }

    void buildDescriptorsSets()
    {
        m_cubeDescriptorSets = MAKE_SHARED(m_cubeDescriptorSets, m_deviceVk);
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
                                         .buffer = m_cubeUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = m_dynamicAlignment } }
        };
        std::map<uint32_t, vk::DescriptorImageInfo> imageInfos{
            { g_textureBinding, vk::DescriptorImageInfo{
                                    .sampler = m_cubeTexture->sampler(),
                                    .imageView = m_cubeTexture->imageView(),
                                    .imageLayout = m_cubeTexture->imageLayout() } }
        };
        m_cubeDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_cubeDescriptorSets->createDescriptorSetLayout(g_textureShaderResource);
        m_cubeDescriptorSets->createDescriptorSets(bufferInfos, imageInfos);

        descriptorPoolSizes = {
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = m_swapchainSize },
            vk::DescriptorPoolSize{
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = m_swapchainSize }
        };
        m_planeDescriptorSets = MAKE_SHARED(m_planeDescriptorSets, m_deviceVk);
        bufferInfos = {
            { g_mvpMatrixUboBinding, vk::DescriptorBufferInfo{
                                         .buffer = m_planeUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } }
        };
        imageInfos = {
            { g_textureBinding, vk::DescriptorImageInfo{
                                    .sampler = m_planeTexture->sampler(),
                                    .imageView = m_planeTexture->imageView(),
                                    .imageLayout = m_planeTexture->imageLayout() } }
        };
        m_planeDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        g_textureShaderResource[0].mode = ShaderResourceMode::Static;
        m_planeDescriptorSets->createDescriptorSetLayout(g_textureShaderResource);
        m_planeDescriptorSets->createDescriptorSets(bufferInfos, imageInfos);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/depth.vert");
        std::string fragShader = getFileContents("shaders/depth.frag");
        m_cubePipeline = MAKE_SHARED(m_cubePipeline, m_deviceVk);
        m_cubePipeline->setProgram(vertSource, fragShader);
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_cubeDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_cubePipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_cubePipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_cubePipeline->setTopology(backend::Topology::Triangles);
        m_cubePipeline->setPipelineLayout(m_cubePipelineLayout);
        m_cubePipeline->setViewport();
        m_cubePipeline->setRasterization();
        m_cubePipeline->setMultisample();
        m_cubePipeline->setDepthStencil(m_depthStencilState->handle());
        m_cubePipeline->setColorBlendAttachment();
        m_cubePipeline->setRenderPass();
        m_cubePipeline->build();

        m_planePipeline = MAKE_SHARED(m_planePipeline, m_deviceVk);
        m_planePipeline->setProgram(vertSource, fragShader);
        pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_planeDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_planePipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_planePipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_planePipeline->setTopology(backend::Topology::Triangles);
        m_planePipeline->setPipelineLayout(m_planePipelineLayout);
        m_planePipeline->setViewport();
        m_planePipeline->setRasterization();
        m_planePipeline->setMultisample();
        m_planePipeline->setDepthStencil(m_depthStencilState->handle());
        m_planePipeline->setColorBlendAttachment();
        m_planePipeline->setRenderPass();
        m_planePipeline->build();
    }

    void update(float deltaTime) override
    {
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        auto beginInfo = vk::CommandBufferBeginInfo{};
        static std::array clearValues = {
            vk::ClearValue{ .color = { .float32 = std::array<float, 4>{ 0.1f, 0.1f, 0.1f, 1.0f } } },
            vk::ClearValue{ .depthStencil = { 1.0f, 0 } }
        };
        static auto renderPassInfo = vk::RenderPassBeginInfo{
            .renderPass = m_deviceVk->renderPass(),
            .renderArea = {
                .offset = { 0, 0 },
                .extent = m_deviceVk->swapchainExtent() },
            .clearValueCount = 2,
            .pClearValues = clearValues.data(),
        };
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            renderPassInfo.setFramebuffer(framebuffer[i]);
            commandBuffers[i].begin(beginInfo);
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_cubePipeline->handle());
            commandBuffers[i].bindVertexBuffers(0, { m_cubeVertexBuffer->buffer() }, { 0 });
            // cubes
            uint32_t dynamicOffset = 0 * static_cast<uint32_t>(m_dynamicAlignment);
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(-1.0f, 0.0f, -1.0f));
            m_cubeUniformBuffer->update(&g_mvpMatrixUbo, m_dynamicAlignment, dynamicOffset);
            commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_cubePipelineLayout, 0, m_cubeDescriptorSets->handle(), dynamicOffset);
            commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVerticesPosTexCoord.size()), 1, 0, 0);

            dynamicOffset = 1 * static_cast<uint32_t>(m_dynamicAlignment);
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(2.0f, 0.0f, 0.0f));
            m_cubeUniformBuffer->update(&g_mvpMatrixUbo, m_dynamicAlignment, dynamicOffset);
            commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_cubePipelineLayout, 0, m_cubeDescriptorSets->handle(), dynamicOffset);
            commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVerticesPosTexCoord.size()), 1, 0, 0);

            // plane
            commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_planePipeline->handle());
            commandBuffers[i].bindVertexBuffers(0, { m_planeVertexBuffer->buffer() }, { 0 });
            g_mvpMatrixUbo.model = glm::mat4(1.0f);
            m_planeUniformBuffer->update(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), 0);
            commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_planePipelineLayout, 0, m_planeDescriptorSets->handle(), nullptr);
            commandBuffers[i].draw(static_cast<std::uint32_t>(g_planeVerticesPosTexCoord.size()), 1, 0, 0);

            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVk* m_render{ nullptr };
    DeviceVk* m_deviceVk{ nullptr };
    std::shared_ptr<DepthStencilStateVk> m_depthStencilState;
    // cube
    std::shared_ptr<PipelineVk> m_cubePipeline;
    std::shared_ptr<BufferVk> m_cubeVertexBuffer;
    std::shared_ptr<BufferVk> m_cubeUniformBuffer;
    std::shared_ptr<TextureVk> m_cubeTexture;
    vk::PipelineLayout m_cubePipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_cubeDescriptorSets;
    // plane
    std::shared_ptr<PipelineVk> m_planePipeline;
    std::shared_ptr<BufferVk> m_planeVertexBuffer;
    std::shared_ptr<BufferVk> m_planeUniformBuffer;
    std::shared_ptr<TextureVk> m_planeTexture;
    vk::PipelineLayout m_planePipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_planeDescriptorSets;

    // 字节对齐
    uint32_t m_swapchainSize{};
    uint32_t m_dynamicAlignment{};
};

void testDepthViewVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 800, 640, "Vulkan Example Depth View" };
    DeviceVk handle(info);
    handle.init();
    GLFWRendererVk renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestDepthViewVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}