//
// Created by cwb on 2022/9/6.
//
#include "../mesh/globalMeshs.h"
#include "commonHandle.h"
#include "deviceVk.h"
#include "engine.h"
#include "glfwRendererVk.h"
#include "pipelineVk.h"
#include "utils.h"

#include <cstddef>

using namespace backend;
namespace
{
class Triangle : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~Triangle() override
    {
        m_device.freeDescriptorSets(m_descriptorPool, m_descriptorSet);
        m_device.destroy(m_descriptorPool);
        m_device.destroy(m_descriptorSetLayout);
        m_device.destroy(m_pipelineLayout);
        m_device.destroyBuffer(m_vertexBuffer);
        m_device.freeMemory(m_deviceMemory);
    }
    void initialize() override
    {
        m_deviceVK = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        m_device = m_deviceVK->handle();
        buildBuffers();
        buildPipeline();
        m_render->initSemaphore();
        m_render->initCommandBuffer();
    }

    void buildBuffers()
    {
        auto dataSize = g_triangleVertex.size() * sizeof(g_triangleVertex[0]);
        // create a vertex buffer for some vertex and color data
        m_vertexBuffer = m_device.createBuffer(vk::BufferCreateInfo{ vk::BufferCreateFlags(), dataSize, vk::BufferUsageFlagBits::eVertexBuffer });
        // allocate device memory for that buffer
        vk::MemoryRequirements memoryRequirements = m_device.getBufferMemoryRequirements(m_vertexBuffer);
        m_deviceMemory = m_device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size,
                                                                        m_deviceVK->getMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)));
        // copy the vertex and color data into that device memory
        auto* pData = static_cast<uint8_t*>(m_device.mapMemory(m_deviceMemory, 0, memoryRequirements.size));
        memcpy(pData, g_triangleVertex.data(), dataSize);
        m_device.unmapMemory(m_deviceMemory);
        // and bind the device memory to the vertex buffer
        m_device.bindBufferMemory(m_vertexBuffer, m_deviceMemory, 0);
        // Vertex bindings and attributes
        m_vertexInputBinding.setBinding(0);
        m_vertexInputBinding.setStride(sizeof(TextureVertex));
        m_vertexInputBinding.setInputRate(vk::VertexInputRate::eVertex);
        m_vertexInputAttribute = {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(TextureVertex, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(TextureVertex, texCoord))
        };
        m_vertexInputInfo = {
            {},
            m_vertexInputBinding,
            m_vertexInputAttribute
        };
    }

    void setupDescriptorSet()
    {
        // Create a single pool to contain data for our descriptor set
        vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, 1);
        m_descriptorPool = m_device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSize));
        // Populate descriptor sets
        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);
        m_descriptorSet = m_device.allocateDescriptorSets(descriptorSetAllocateInfo).front();
    }

    void setupDescriptorLayout()
    {
        // create a DescriptorSetLayout
        // set binding：也就是gl里面的uniform变量
        // 此处没有传uniform变量，直接传空即可
        // set layout:也就是opengl里面的vertex attributes
        m_descriptorSetLayout = m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), {}));
        // create a PipelineLayout using that DescriptorSetLayout
        m_pipelineLayout = m_device.createPipelineLayout({ vk::PipelineLayoutCreateFlags(), m_descriptorSetLayout });
    }

    void buildPipeline()
    {
        setupDescriptorLayout();
        setupDescriptorSet();
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVK);
        m_pipeline->setProgram(vertSource, fragShader);
        m_pipeline->initVertexBuffer(m_vertexInputInfo);
        m_pipeline->setAssembly();
        m_pipeline->setPipelineLayout(m_pipelineLayout);
        m_pipeline->setViewport();
        m_pipeline->setRasterization();
        m_pipeline->setMultisample();
        m_pipeline->setDepthStencil();
        m_pipeline->setColorBlendAttachment();
        m_pipeline->setRenderPass();
        m_pipeline->build();
        m_render->setPipeline(m_pipeline);
    }

    void render() override
    {
        uint32_t width = m_deviceVK->width();
        uint32_t height = m_deviceVK->height();
        auto& commandBuffers = m_render->commandBuffers();
        auto& frameBuffers = m_render->frameBuffers();
        vk::ClearValue clearValues{};
        static float red = 1.0f;
        //        red = red > 1.0f ? 0.0f : red + 0.005f;
        clearValues.color.setFloat32({ red, 0.0f, 0.0f, 1.0f });
        vk::RenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.setRenderPass(m_pipeline->renderPass());
        renderPassBeginInfo.setRenderArea({ { 0, 0 }, { width, height } });
        renderPassBeginInfo.setClearValueCount(1);
        renderPassBeginInfo.setClearValues(clearValues);
        vk::Viewport viewport{ 0, 0, (float)width, (float)height, 0.0f, 1.0f };
        vk::Rect2D scissor{ { 0, 0 }, { width, height } };
        for (size_t i = 0; i < commandBuffers.size(); i++)
        {
            renderPassBeginInfo.framebuffer = frameBuffers[i];
            auto beginInfo = vk::CommandBufferBeginInfo{};
            beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            commandBuffers[i].begin(beginInfo);
            commandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
            commandBuffers[i].setViewport(0, 1, &viewport);
            commandBuffers[i].setScissor(0, 1, &scissor);
            commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSet, nullptr);
            commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->handle()); // 等价于 opengl的 bind program 和一设定些状态
            commandBuffers[i].bindVertexBuffers(0, m_vertexBuffer, { 0 });
            commandBuffers[i].draw(3, 1, 0, 0);
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVK{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    vk::Device m_device;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    vk::Buffer m_vertexBuffer;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::PipelineLayout m_pipelineLayout;
    vk::VertexInputBindingDescription m_vertexInputBinding;
    std::array<vk::VertexInputAttributeDescription, 2> m_vertexInputAttribute;

    vk::DescriptorPool m_descriptorPool;
    vk::DescriptorSet m_descriptorSet;
    vk::DeviceMemory m_deviceMemory;
};
} // namespace

void triangleVk()
{
    Device::Info info{ Device::RenderType::Vulkan, 640, 480, "Vulkan Triangle" };
    DeviceVK handle(info);
    handle.init();
    GLFWRendererVK renderer(&handle);
    Engine engine(renderer);
    auto effect = std::make_shared<Triangle>(&renderer);
    engine.setEffect(effect);
    engine.run();
}
