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

using namespace backend;
namespace
{
class Triangle : public EffectBase
{
public:
    using EffectBase::EffectBase;
    ~Triangle() override
    {
        m_device.destroyBuffer(m_vertexBuffer);
    }
    void initialize() override
    {
        m_deviceVK = dynamic_cast<DeviceVK*>(m_renderer->device());
        m_render = dynamic_cast<GLFWRendererVK*>(m_renderer);
        m_device = m_deviceVK->handle();
        buildPipeline();
        buildBuffers();
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
        vk::DeviceMemory deviceMemory = m_device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, 0));
        // copy the vertex and color data into that device memory
        auto* pData = static_cast<uint8_t*>(m_device.mapMemory(deviceMemory, 0, memoryRequirements.size));
        memcpy(pData, g_triangleVertex.data(), dataSize);
        m_device.unmapMemory(deviceMemory);
        // and bind the device memory to the vertex buffer
        m_device.bindBufferMemory(m_vertexBuffer, deviceMemory, 0);
        // ===
        vk::VertexInputBindingDescription vertexInputBinding{};
        vertexInputBinding.setBinding(0);
        vertexInputBinding.setStride(sizeof(glm::vec4) * 2);
        vertexInputBinding.setInputRate(vk::VertexInputRate::eVertex);
        std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttribute = {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, 0),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, sizeof(float) * 4)
        };
        m_vertexInputInfo = {
            {},
            vertexInputBinding,
            vertexInputAttribute
        };
    }

    void buildPipeline()
    {
        std::string vertSource = getFileContents("shaders/triangle.vert");
        std::string fragShader = getFileContents("shaders/triangle.frag");
        m_pipeline = MAKE_SHARED(m_pipeline, m_deviceVK);
        m_pipeline->setProgram(vertSource, fragShader);
        m_pipeline->initVertexBuffer(m_vertexInputInfo);
        m_pipeline->setAssembly();
        // create a DescriptorSetLayout
        vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
        vk::DescriptorSetLayout descriptorSetLayout = m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), descriptorSetLayoutBinding));
        // create a PipelineLayout using that DescriptorSetLayout
        m_pipelineLayout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout));
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
        m_render->commit(m_vertexBuffer);
    }

private:
    GLFWRendererVK* m_render{ nullptr };
    DeviceVK* m_deviceVK{ nullptr };
    std::shared_ptr<PipelineVk> m_pipeline;
    vk::Device m_device;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
    vk::Buffer m_vertexBuffer;
    vk::PipelineLayout m_pipelineLayout;
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
