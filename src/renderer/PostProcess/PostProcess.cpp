#include <renderer/PostProcess/PostProcess.h>
#include <renderer/Pipeline/Pipeline.h>
#include <core/Logger.h>

#include <postprocess.vert.h>
#include <postprocess.frag.h>

namespace synapse {

PostProcess::PostProcess(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                         VkImageView inputView, VkSampler inputSampler)
    : m_Device(device)
{
    CreateDescriptors(inputView, inputSampler);

    const VkDescriptorSetLayout layouts[] = { m_DescriptorSetLayout };
    m_Pipeline = std::make_unique<Pipeline>(
        m_Device, renderPass, extent, layouts, 1,
        std::string_view(reinterpret_cast<const char*>(synapse::postprocess_vert_data),
                         synapse::postprocess_vert_size),
        std::string_view(reinterpret_cast<const char*>(synapse::postprocess_frag_data),
                         synapse::postprocess_frag_size),
        0, SYNAPSE_SHADER_DIR "/postprocess.vert.glsl", SYNAPSE_SHADER_DIR "/postprocess.frag.glsl",
        VK_SAMPLE_COUNT_1_BIT, 1
    );

    SYNAPSE_CORE_INFO("Post-processing system initialized");
}

PostProcess::~PostProcess()
{
    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
    }
    if (m_DescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
    }
}

void PostProcess::CreateDescriptors(VkImageView inputView, VkSampler inputSampler)
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    {
        SYNAPSE_CORE_ERROR("PostProcess: Failed to create descriptor set layout");
        return;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        SYNAPSE_CORE_ERROR("PostProcess: Failed to create descriptor pool");
        return;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayout;
    if (vkAllocateDescriptorSets(m_Device, &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
    {
        SYNAPSE_CORE_ERROR("PostProcess: Failed to allocate descriptor sets");
        return;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = inputView;
    imageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_DescriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(m_Device, 1, &write, 0, nullptr);
}

void PostProcess::Render(VkCommandBuffer commandBuffer, VkExtent2D extent)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetHandle());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetLayout(),
                            0, 1, &m_DescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void PostProcess::ReloadIfChanged()
{
    m_Pipeline->ReloadIfChanged();
}

} // namespace synapse
