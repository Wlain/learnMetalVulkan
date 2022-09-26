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
vk::VertexInputBindingDescription getBindingDescription()
{
    auto bindingDescription = vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(TriangleVertex),
        .inputRate = vk::VertexInputRate::eVertex
    };

    return bindingDescription;
}

std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
{
    auto attributeDescriptions = std::array<vk::VertexInputAttributeDescription, 2>{
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(TriangleVertex, position) },
        vk::VertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = offsetof(TriangleVertex, color) },
    };

    return attributeDescriptions;
}

namespace
{
class Triangle : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~Triangle() override
    {
        m_device.destroy(m_descriptorPool);
        m_device.destroy(m_descriptorSetLayout);
        m_device.destroy(m_pipelineLayout);
        m_device.destroyBuffer(m_vertexBuffer);
        m_device.freeMemory(m_vertexBufferMemory);
    }
    void initialize() override
    {
        m_deviceVK = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        m_device = m_deviceVK->handle();
        buildPipeline();
        m_render->createFrameBuffers();
        m_render->createCommandPool();
        buildBuffers();
        m_render->createCommandBuffers();
        m_render->createSyncObjects();
    }

    std::pair<vk::Buffer, vk::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        auto bufferInfo = vk::BufferCreateInfo{
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        auto buffer = m_device.createBuffer(bufferInfo);

        auto memoryRequirements = m_device.getBufferMemoryRequirements(buffer);
        auto allocInfo = vk::MemoryAllocateInfo{
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = m_deviceVK->findMemoryType(memoryRequirements.memoryTypeBits, properties)
        };
        auto bufferMemory = m_device.allocateMemory(allocInfo);
        m_device.bindBufferMemory(buffer, bufferMemory, 0);
        return { buffer, bufferMemory };
    }

    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        auto cmdPool = m_render->commandPool();
        auto allocInfo = vk::CommandBufferAllocateInfo{
            .commandPool = cmdPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };

        auto commandBuffers = m_device.allocateCommandBuffers(allocInfo);

        auto beginInfo = vk::CommandBufferBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };

        commandBuffers[0].begin(beginInfo);
        auto copyRegion = vk::BufferCopy{ .size = size };
        commandBuffers[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
        commandBuffers[0].end();

        auto submitInfo = vk::SubmitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers = commandBuffers.data()
        };
        m_deviceVK->graphicsQueue().submit(submitInfo);
        m_deviceVK->graphicsQueue().waitIdle();
        m_device.freeCommandBuffers(cmdPool, 1, commandBuffers.data());
    }

    void buildBuffers()
    {
        vk::DeviceSize bufferSize = g_triangleVertex.size() * sizeof(g_triangleVertex[0]);
        // create a vertex buffer for some vertex and color data

        auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize,
                                                                 vk::BufferUsageFlagBits::eTransferSrc,
                                                                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        // copy the vertex and color data into that device memory
        auto* pData = (m_device.mapMemory(stagingBufferMemory, {}, bufferSize, {}));
        memcpy(pData, g_triangleVertex.data(), static_cast<std::size_t>(bufferSize));
        m_device.unmapMemory(stagingBufferMemory);

        std::tie(m_vertexBuffer, m_vertexBufferMemory) = createBuffer(bufferSize,
                                                                      vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                                                                      vk::MemoryPropertyFlagBits::eDeviceLocal);
        copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
        m_render->setVertexBuffer(m_vertexBuffer);
        m_device.destroyBuffer(stagingBuffer);
        m_device.freeMemory(stagingBufferMemory);
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVK);
        m_pipeline->setProgram(vertSource, fragShader);

        ///
        m_bindingDescription = getBindingDescription();
        m_attributeDescriptions = getAttributeDescriptions();
        m_vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &m_bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size()),
            .pVertexAttributeDescriptions = m_attributeDescriptions.data()
        };
        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0
        };
        m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);
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

    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVK{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    vk::Device m_device;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    std::array<vk::VertexInputAttributeDescription, 2> m_attributeDescriptions;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::PipelineLayout m_pipelineLayout;
    vk::VertexInputBindingDescription m_bindingDescription;
    std::array<vk::VertexInputAttributeDescription, 2> m_vertexInputAttribute;

    vk::DescriptorPool m_descriptorPool;
    vk::Buffer m_vertexBuffer;
    vk::DeviceMemory m_vertexBufferMemory;
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
