//
// Created by william on 2022/10/31.
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

#include <array>
using namespace backend;

namespace
{
class TestLightingColorsVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestLightingColorsVk() override
    {
        auto device = m_deviceVk->handle();
        device.destroy(m_lightCubeDescriptorSetLayout);
        device.destroy(m_colorDescriptorSetLayout);
        device.freeDescriptorSets(m_lightCubeDescriptorPool, m_lightCubeDescriptorSets);
        device.freeDescriptorSets(m_colorDescriptorPool, m_colorDescriptorSets);
        device.destroy(m_lightCubeDescriptorPool);
        device.destroy(m_colorDescriptorPool);
    }

    void initialize() override
    {
        m_deviceVk = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_swapchainSize = (uint32_t)m_deviceVk->swapchainImageViews().size();
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        buildBuffers();
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
        EffectBase::update(deltaTime);
        g_mvpMatrixUbo.view = m_camera.viewMatrix();
        g_mvpMatrixUbo.proj = glm::perspective(glm::radians(m_camera.zoom), (float)m_width / (float)m_height, 0.1f, 100.0f);
    }

    vk::DescriptorSetLayout& createLightCubeDescriptorSetLayout()
    {
        if (!m_lightCubeDescriptorSetLayout)
        {
            auto vertUboLayoutBinding = vk::DescriptorSetLayoutBinding{
                .binding = g_mvpMatrixUboBinding,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .pImmutableSamplers = nullptr // optional (only relevant to Image Sampling;
            };
            std::array binding = { vertUboLayoutBinding };
            auto layoutInfo = vk::DescriptorSetLayoutCreateInfo{
                .bindingCount = binding.size(),
                .pBindings = binding.data()
            };
            m_lightCubeDescriptorSetLayout = m_deviceVk->handle().createDescriptorSetLayout(layoutInfo);
        }
        return m_lightCubeDescriptorSetLayout;
    }

    vk::DescriptorSetLayout& createColorDescriptorSetLayout()
    {
        if (!m_colorDescriptorSetLayout)
        {
            auto vertUboLayoutBinding = vk::DescriptorSetLayoutBinding{
                .binding = g_mvpMatrixUboBinding,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .pImmutableSamplers = nullptr // optional (only relevant to Image Sampling;
            };
            auto fragUboLayoutBinding = vk::DescriptorSetLayoutBinding{
                .binding = g_lightingColorUboBinding,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
                .pImmutableSamplers = nullptr // optional (only relevant to Image Sampling;
            };

            std::array binding = { vertUboLayoutBinding, fragUboLayoutBinding };
            auto layoutInfo = vk::DescriptorSetLayoutCreateInfo{
                .bindingCount = binding.size(),
                .pBindings = binding.data()
            };
            m_colorDescriptorSetLayout = m_deviceVk->handle().createDescriptorSetLayout(layoutInfo);
        }
        return m_colorDescriptorSetLayout;
    }

    vk::DescriptorPool& createLightCubeDescriptorPool()
    {
        if (!m_lightCubeDescriptorPool)
        {
            std::array poolSizes{
                vk::DescriptorPoolSize{
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = m_swapchainSize }
            };
            auto poolInfo = vk::DescriptorPoolCreateInfo{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = m_swapchainSize,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data()
            };
            m_lightCubeDescriptorPool = m_deviceVk->handle().createDescriptorPool(poolInfo);
        }
        return m_lightCubeDescriptorPool;
    }

    vk::DescriptorPool& createColorDescriptorPool()
    {
        if (!m_colorDescriptorPool)
        {
            std::array poolSizes{
                vk::DescriptorPoolSize{
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = m_swapchainSize },
                vk::DescriptorPoolSize{
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = m_swapchainSize }
            };
            auto poolInfo = vk::DescriptorPoolCreateInfo{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = m_swapchainSize,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data()
            };
            m_colorDescriptorPool = m_deviceVk->handle().createDescriptorPool(poolInfo);
        }
        return m_colorDescriptorPool;
    }

    std::vector<vk::DescriptorSet>& createLightCubeDescriptorSets()
    {
        if (m_lightCubeDescriptorSets.empty())
        {
            std::vector<vk::DescriptorSetLayout> layouts(m_swapchainSize, createLightCubeDescriptorSetLayout());
            auto allocInfo = vk::DescriptorSetAllocateInfo{
                .descriptorPool = createLightCubeDescriptorPool(),
                .descriptorSetCount = m_swapchainSize,
                .pSetLayouts = layouts.data()
            };
            m_lightCubeDescriptorSets.resize(m_swapchainSize);
            m_lightCubeDescriptorSets = m_deviceVk->handle().allocateDescriptorSets(allocInfo);
            for (size_t i = 0; i < m_swapchainSize; i++)
            {
                auto vertBufferInfo = vk::DescriptorBufferInfo{
                    .buffer = m_vertUniformBuffer->buffer(),
                    .offset = 0,
                    .range = sizeof(VertMVPMatrixUBO)
                };
                std::array descriptorWrites = {
                    vk::WriteDescriptorSet{
                        .dstSet = m_lightCubeDescriptorSets[i],
                        .dstBinding = g_mvpMatrixUboBinding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                        .pBufferInfo = &vertBufferInfo }
                };
                m_deviceVk->handle().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
        return m_lightCubeDescriptorSets;
    }

    std::vector<vk::DescriptorSet>& createColorDescriptorSets()
    {
        if (m_colorDescriptorSets.empty())
        {
            std::vector<vk::DescriptorSetLayout> layouts(m_swapchainSize, createColorDescriptorSetLayout());

            auto allocInfo = vk::DescriptorSetAllocateInfo{
                .descriptorPool = createColorDescriptorPool(),
                .descriptorSetCount = m_swapchainSize,
                .pSetLayouts = layouts.data()
            };
            m_colorDescriptorSets.resize(m_swapchainSize);
            m_colorDescriptorSets = m_deviceVk->handle().allocateDescriptorSets(allocInfo);
            for (size_t i = 0; i < m_swapchainSize; i++)
            {
                auto vertBufferInfo = vk::DescriptorBufferInfo{
                    .buffer = m_vertUniformBuffer->buffer(),
                    .offset = 0,
                    .range = sizeof(VertMVPMatrixUBO)
                };
                auto fragBufferInfo = vk::DescriptorBufferInfo{
                    .buffer = m_fragUniformBuffer->buffer(),
                    .offset = 0,
                    .range = sizeof(FragLightingColorUBO)
                };
                std::array descriptorWrites = {
                    vk::WriteDescriptorSet{
                        .dstSet = m_colorDescriptorSets[i],
                        .dstBinding = g_mvpMatrixUboBinding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                        .pBufferInfo = &vertBufferInfo },
                    vk::WriteDescriptorSet{
                        .dstSet = m_colorDescriptorSets[i],
                        .dstBinding = g_lightingColorUboBinding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                        .pBufferInfo = &fragBufferInfo }
                };
                m_deviceVk->handle().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
        return m_colorDescriptorSets;
    }

    void buildPipeline()
    {
        // lightCube
        std::string vertSource = getFileContents("shaders/lightCube.vert");
        std::string fragShader = getFileContents("shaders/lightCube.frag");
        m_lightCubePipeline = MAKE_SHARED(m_lightCubePipeline, m_deviceVk);
        m_lightCubePipeline->setProgram(vertSource, fragShader);
        m_lightCubeBindingDescription = getPosBindingDescription();
        m_lightCubeAttributeDescriptions = getPosAttributeDescriptions();
        m_lightCubeVertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &m_lightCubeBindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_lightCubeAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = m_lightCubeAttributeDescriptions.data()
        };
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &createLightCubeDescriptorSetLayout(),
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
        m_lightCubePipeline->initVertexBuffer(m_lightCubeVertexInputInfo);
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
        m_colorPipeline = MAKE_SHARED(m_colorPipeline, m_render->device());
        m_colorPipeline->setProgram(vertSource, fragShader);
        m_colorBindingDescription = getPosBindingDescription();
        m_colorAttributeDescriptions = getPosAttributeDescriptions();
        m_colorVertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &m_colorBindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_colorAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = m_colorAttributeDescriptions.data()
        };
        pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &createColorDescriptorSetLayout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_colorPipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_colorPipeline->initVertexBuffer(m_colorVertexInputInfo);
        m_colorPipeline->setTopology(backend::Topology::Triangles);
        m_colorPipeline->setPipelineLayout(m_colorPipelineLayout);
        m_colorPipeline->setViewport();
        m_colorPipeline->setRasterization();
        m_colorPipeline->setMultisample();
        m_colorPipeline->setDepthStencil(m_depthStencilState);
        m_colorPipeline->setColorBlendAttachment();
        m_colorPipeline->setRenderPass();
        m_colorPipeline->build();
    }

    void render() override
    {
        auto& commandBuffers = m_deviceVk->commandBuffers();
        auto& framebuffer = m_deviceVk->swapchainFramebuffers();
        for (std::size_t i = 0; i < commandBuffers.size(); ++i)
        {
            auto beginInfo = vk::CommandBufferBeginInfo{};
            commandBuffers[i].begin(beginInfo);
            std::array<vk::ClearValue, 2> clearValues = {
                vk::ClearValue{
                    .color = { .float32 = std::array<float, 4>{ 1.0f, 0.0f, 0.0f, 1.0f } } },
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
                .pClearValues = clearValues.data(),
            };

            {
                commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
                commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_lightCubePipeline->handle());
                commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
                // calculate the model matrix for each object and pass it to shader before drawing
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, g_lightPos);
                g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.2f)); // a smaller cube
                m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
                commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_lightCubePipelineLayout, 0, createLightCubeDescriptorSets()[i], nullptr);
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_cubeVertices.size()), 1, 0, 0);
                commandBuffers[i].endRenderPass();
            }
            {
                commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
                commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_colorPipeline->handle());
                commandBuffers[i].bindVertexBuffers(0, { m_vertexBuffer->buffer() }, { 0 });
                // calculate the model matrix for each object and pmass it to shader before drawing
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                m_vertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(VertMVPMatrixUBO), 0);
                commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_colorPipelineLayout, 0, createColorDescriptorSets()[i], nullptr);
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
    std::shared_ptr<PipelineVk> m_colorPipeline;
    std::shared_ptr<BufferVK> m_vertexBuffer;
    std::shared_ptr<BufferVK> m_vertUniformBuffer;
    std::shared_ptr<BufferVK> m_fragUniformBuffer;
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilState;
    vk::PipelineVertexInputStateCreateInfo m_lightCubeVertexInputInfo;
    vk::PipelineVertexInputStateCreateInfo m_colorVertexInputInfo;

    std::array<vk::VertexInputAttributeDescription, 1> m_lightCubeAttributeDescriptions;
    std::array<vk::VertexInputAttributeDescription, 1> m_colorAttributeDescriptions;
    vk::PipelineLayout m_lightCubePipelineLayout;
    vk::PipelineLayout m_colorPipelineLayout;
    vk::VertexInputBindingDescription m_lightCubeBindingDescription;
    vk::VertexInputBindingDescription m_colorBindingDescription;
    vk::DescriptorSetLayout m_lightCubeDescriptorSetLayout;
    vk::DescriptorSetLayout m_colorDescriptorSetLayout;
    vk::DescriptorPool m_lightCubeDescriptorPool;
    vk::DescriptorPool m_colorDescriptorPool;
    std::vector<vk::DescriptorSet> m_lightCubeDescriptorSets;
    std::vector<vk::DescriptorSet> m_colorDescriptorSets;
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