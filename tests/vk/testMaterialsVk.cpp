//
// Created by william on 2022/11/28.
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
class TestMaterialsVk : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~TestMaterialsVk() override = default;

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
        m_lightCubeVertexBuffer->create(g_sphereMesh.size() * sizeof(g_sphereMesh[0]), (void*)g_sphereMesh.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_lightCubeVertUniformBuffer = MAKE_SHARED(m_lightCubeVertUniformBuffer, m_deviceVk);
        m_lightCubeVertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_lightCubeFragUniformBuffer = MAKE_SHARED(m_lightCubeFragUniformBuffer, m_deviceVk);
        m_lightCubeFragUniformBuffer->create(sizeof(g_lightColorUbo), (void*)&g_lightColorUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);

        m_materialsVertexBuffer = MAKE_SHARED(m_materialsVertexBuffer, m_deviceVk);
        m_materialsVertexBuffer->create(g_cubeVerticesWithNormal.size() * sizeof(g_cubeVerticesWithNormal[0]), (void*)g_cubeVerticesWithNormal.data(), Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::VertexBuffer);
        m_materialsVertUniformBuffer = MAKE_SHARED(m_materialsVertUniformBuffer, m_deviceVk);
        m_materialsVertUniformBuffer->create(sizeof(g_mvpMatrixUbo), (void*)&g_mvpMatrixUbo, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
        m_materialsFragUniformBuffer = MAKE_SHARED(m_materialsFragUniformBuffer, m_deviceVk);
        m_materialsFragUniformBuffer->create(sizeof(g_fragMaterialsColorUBO), (void*)&g_fragMaterialsColorUBO, Buffer::BufferUsage::DynamicDraw, Buffer::BufferType::UniformBuffer);
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
                .descriptorCount = m_swapchainSize },

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

        m_materialsDescriptorSets = MAKE_SHARED(m_materialsDescriptorSets, m_deviceVk);
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
                                         .buffer = m_materialsVertUniformBuffer->buffer(),
                                         .offset = 0,
                                         .range = sizeof(VertMVPMatrixUBO) } },
            { g_materialsUboBinding, vk::DescriptorBufferInfo{ .buffer = m_materialsFragUniformBuffer->buffer(), .offset = 0, .range = sizeof(FragMaterialsColorUBO) } },
        };
        m_materialsDescriptorSets->createDescriptorPool(descriptorPoolSizes, m_swapchainSize);
        m_materialsDescriptorSets->createDescriptorSetLayout(g_materialsShaderResource);
        m_materialsDescriptorSets->createDescriptorSets(bufferInfos, {});
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
        vertSource = getFileContents("shaders/materials.vert");
        fragShader = getFileContents("shaders/materials.frag");
        m_materialsPipeline = MAKE_SHARED(m_materialsPipeline, m_render->device());
        m_materialsPipeline->setProgram(vertSource, fragShader);
        pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &m_materialsDescriptorSets->layout(),
            .pushConstantRangeCount = 0,   // optional
            .pPushConstantRanges = nullptr // optional
        };
        m_materialsPipelineLayout = m_deviceVk->handle().createPipelineLayout(pipelineLayoutInfo);
        m_materialsPipeline->setAttributeDescription(getTwoElemsAttributesDescriptions());
        m_materialsPipeline->setTopology(backend::Topology::Triangles);
        m_materialsPipeline->setPipelineLayout(m_materialsPipelineLayout);
        m_materialsPipeline->setViewport();
        m_materialsPipeline->setRasterization();
        m_materialsPipeline->setMultisample();
        m_materialsPipeline->setDepthStencil(m_depthStencilState->handle());
        m_materialsPipeline->setColorBlendAttachment();
        m_materialsPipeline->setRenderPass();
        m_materialsPipeline->build();
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
                g_fragMaterialsColorUBO.light.position.x = 1.0f + sin(m_duringTime) * 2.0f;
                g_fragMaterialsColorUBO.light.position.y = sin(m_duringTime / 2.0f) * 1.0f;
                // calculate the model matrix for each object and pass it to shader before drawing
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                g_mvpMatrixUbo.model = glm::translate(g_mvpMatrixUbo.model, glm::vec3(g_fragMaterialsColorUBO.light.position));
                g_mvpMatrixUbo.model = glm::scale(g_mvpMatrixUbo.model, glm::vec3(0.05f)); // a smaller cube
                m_lightCubeVertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), 0);
                g_lightColorUbo.lightColor.x = static_cast<float>(std::abs(sin(m_duringTime * 2.0)));
                g_lightColorUbo.lightColor.y = static_cast<float>(std::abs(sin(m_duringTime * 0.7)));
                g_lightColorUbo.lightColor.z = static_cast<float>(std::abs(sin(m_duringTime * 1.3)));
                m_lightCubeFragUniformBuffer->update(&g_lightColorUbo, sizeof(g_lightColorUbo), 0);
                DeviceVK::bindPipeline()(commandBuffers[i], m_lightCubePipeline->handle(), m_lightCubePipelineLayout, m_lightCubeDescriptorSets->handle());
                commandBuffers[i].draw(static_cast<std::uint32_t>(g_sphereMesh.size()), 1, 0, 0);
            }
            // draw materials cube
            {
                commandBuffers[i].bindVertexBuffers(0, { m_materialsVertexBuffer->buffer() }, { 0 });
                g_mvpMatrixUbo.model = glm::mat4(1.0f);
                m_materialsVertUniformBuffer->update(&g_mvpMatrixUbo, sizeof(g_mvpMatrixUbo), 0);
                g_fragMaterialsColorUBO.viewPos = glm::vec4(m_camera.position, 1.0f);
                g_fragMaterialsColorUBO.light.diffuse = g_lightColorUbo.lightColor * glm::vec4(0.5f);            // decrease the influence;
                g_fragMaterialsColorUBO.light.ambient = g_fragMaterialsColorUBO.light.diffuse * glm::vec4(0.2f); // decrease the influence
                g_fragMaterialsColorUBO.light.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
                m_materialsFragUniformBuffer->update(&g_fragMaterialsColorUBO, sizeof(g_fragMaterialsColorUBO), 0);
                DeviceVK::bindPipeline()(commandBuffers[i], m_materialsPipeline->handle(), m_materialsPipelineLayout, m_materialsDescriptorSets->handle());
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
    std::shared_ptr<BufferVK> m_lightCubeFragUniformBuffer;
    vk::PipelineLayout m_lightCubePipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_lightCubeDescriptorSets;

    std::shared_ptr<PipelineVk> m_materialsPipeline;
    std::shared_ptr<BufferVK> m_materialsVertexBuffer;
    std::shared_ptr<BufferVK> m_materialsVertUniformBuffer;
    std::shared_ptr<BufferVK> m_materialsFragUniformBuffer;
    vk::PipelineLayout m_materialsPipelineLayout;
    std::shared_ptr<DescriptorSetVk> m_materialsDescriptorSets;
    uint32_t m_swapchainSize{};
};
} // namespace

void testMaterialsVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 800, 640, "Vulkan Example materials" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<TestMaterialsVk>(&renderer);
    engine.setEffect(effect);
    engine.run();
}