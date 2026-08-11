#include <renderer/Environment/SkyboxPass.h>

#include <core/Logger.h>

#include <skybox.vert.h>
#include <skybox.frag.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace synapse {
namespace {

#define SKYBOX_VK_CHECK(expr)                                                   \
    do                                                                          \
    {                                                                           \
        const VkResult result = (expr);                                         \
        if (result != VK_SUCCESS)                                               \
        {                                                                       \
            SYNAPSE_CORE_CRITICAL("SkyboxPass: Vulkan error 0x{:x} at {}:{}",  \
                                  static_cast<u32>(result), __FILE__, __LINE__);\
            std::abort();                                                       \
        }                                                                       \
    } while (false)

constexpr u32 kLayerCount = 6;

u32 FindMemoryType(VkPhysicalDevice physicalDevice, u32 typeBits,
                   VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeBits & (1u << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    SYNAPSE_CORE_CRITICAL("SkyboxPass: no suitable memory type");
    std::abort();
}

glm::vec3 CubeFaceDirection(u32 layer, float u, float v)
{
    const float x = u * 2.0f - 1.0f;
    const float y = v * 2.0f - 1.0f;
    switch (layer)
    {
        case 0: return glm::vec3(1.0f, -y, -x);
        case 1: return glm::vec3(-1.0f, -y, x);
        case 2: return glm::vec3(x, 1.0f, y);
        case 3: return glm::vec3(x, -1.0f, -y);
        case 4: return glm::vec3(x, -y, 1.0f);
        default: return glm::vec3(-x, -y, -1.0f);
    }
}

glm::vec3 SkyColor(const glm::vec3& dir, const glm::vec3& sunDir)
{
    const glm::vec3 zenith(0.05f, 0.12f, 0.35f);
    const glm::vec3 horizon(0.62f, 0.76f, 0.92f);

    const float f = std::clamp(dir.y * 0.5f + 0.5f, 0.0f, 1.0f);
    glm::vec3 color = glm::mix(horizon, zenith, std::pow(f, 1.5f));

    if (glm::dot(glm::normalize(dir), sunDir) > 0.996f)
    {
        color = glm::vec3(1.0f, 0.96f, 0.85f);
    }
    return color;
}

VkCommandBuffer BeginSingleTime(VkDevice device, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    SKYBOX_VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    SKYBOX_VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
    return cmd;
}

void EndSingleTime(VkDevice device, VkQueue queue, VkCommandPool pool, VkCommandBuffer cmd)
{
    SKYBOX_VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    SKYBOX_VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    SKYBOX_VK_CHECK(vkQueueWaitIdle(queue));

    vkFreeCommandBuffers(device, pool, 1, &cmd);
}

} // namespace

SkyboxPass::SkyboxPass(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass,
                       u32 queueFamilyIndex)
    : m_Device(device), m_PhysicalDevice(physicalDevice), m_QueueFamilyIndex(queueFamilyIndex)
{
    CreateCubeMap();
    CreateDescriptors();
    CreatePipeline(renderPass);

    SYNAPSE_CORE_INFO("Skybox created ({}x{} cube map)", kFaceSize, kFaceSize);
}

SkyboxPass::~SkyboxPass()
{
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device, m_SetLayout, nullptr);
    vkDestroySampler(m_Device, m_Sampler, nullptr);
    vkDestroyImageView(m_Device, m_ImageView, nullptr);
    vkDestroyImage(m_Device, m_Image, nullptr);
    vkFreeMemory(m_Device, m_Memory, nullptr);
}

void SkyboxPass::CreateCubeMap()
{
    constexpr u32 kBytesPerLayer = kFaceSize * kFaceSize * 4;
    constexpr u32 kTotalBytes = kBytesPerLayer * kLayerCount;

    const glm::vec3 sunDir = glm::normalize(glm::vec3(0.3f, 1.0f, 0.4f));

    std::array<u8, kTotalBytes> pixels{};
    for (u32 layer = 0; layer < kLayerCount; ++layer)
    {
        for (u32 y = 0; y < kFaceSize; ++y)
        {
            for (u32 x = 0; x < kFaceSize; ++x)
            {
                const glm::vec3 dir = CubeFaceDirection(
                    layer, (static_cast<float>(x) + 0.5f) / kFaceSize,
                    (static_cast<float>(y) + 0.5f) / kFaceSize);
                const glm::vec3 color = SkyColor(dir, sunDir);

                const u32 offset = layer * kBytesPerLayer + (y * kFaceSize + x) * 4;
                pixels[offset + 0] = static_cast<u8>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
                pixels[offset + 1] = static_cast<u8>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
                pixels[offset + 2] = static_cast<u8>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
                pixels[offset + 3] = 255;
            }
        }
    }

    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = m_QueueFamilyIndex;
    SKYBOX_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &pool));

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_Device, m_QueueFamilyIndex, 0, &queue);

    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = kTotalBytes;
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    SKYBOX_VK_CHECK(vkCreateBuffer(m_Device, &stagingInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_Device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(m_PhysicalDevice, memRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    SKYBOX_VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &stagingMemory));
    SKYBOX_VK_CHECK(vkBindBufferMemory(m_Device, stagingBuffer, stagingMemory, 0));

    void* mapped = nullptr;
    vkMapMemory(m_Device, stagingMemory, 0, kTotalBytes, 0, &mapped);
    std::memcpy(mapped, pixels.data(), kTotalBytes);
    vkUnmapMemory(m_Device, stagingMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent = {kFaceSize, kFaceSize, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = kLayerCount;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    SKYBOX_VK_CHECK(vkCreateImage(m_Device, &imageInfo, nullptr, &m_Image));

    vkGetImageMemoryRequirements(m_Device, m_Image, &memRequirements);
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(m_PhysicalDevice, memRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    SKYBOX_VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory));
    SKYBOX_VK_CHECK(vkBindImageMemory(m_Device, m_Image, m_Memory, 0));

    VkCommandBuffer cmd = BeginSingleTime(m_Device, pool);

    VkImageMemoryBarrier toTransfer{};
    toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image = m_Image;
    toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toTransfer.subresourceRange.levelCount = 1;
    toTransfer.subresourceRange.layerCount = kLayerCount;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toTransfer);

    std::array<VkBufferImageCopy, kLayerCount> regions{};
    for (u32 layer = 0; layer < kLayerCount; ++layer)
    {
        regions[layer].bufferOffset = layer * kBytesPerLayer;
        regions[layer].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[layer].imageSubresource.baseArrayLayer = layer;
        regions[layer].imageSubresource.layerCount = 1;
        regions[layer].imageExtent = {kFaceSize, kFaceSize, 1};
    }
    vkCmdCopyBufferToImage(cmd, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           kLayerCount, regions.data());

    VkImageMemoryBarrier toShaderRead{};
    toShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toShaderRead.image = m_Image;
    toShaderRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toShaderRead.subresourceRange.levelCount = 1;
    toShaderRead.subresourceRange.layerCount = kLayerCount;
    toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toShaderRead);

    EndSingleTime(m_Device, queue, pool, cmd);
    vkDestroyCommandPool(m_Device, pool, nullptr);
    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingMemory, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = kLayerCount;
    SKYBOX_VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView));

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxLod = 1.0f;
    SKYBOX_VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler));
}

void SkyboxPass::CreateDescriptors()
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    SKYBOX_VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_SetLayout));

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    SKYBOX_VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_Pool));

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_Pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_SetLayout;
    SKYBOX_VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_Set));

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_ImageView;
    imageInfo.sampler = m_Sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_Set;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(m_Device, 1, &write, 0, nullptr);
}

void SkyboxPass::CreatePipeline(VkRenderPass renderPass)
{
    const auto* vertData = reinterpret_cast<const char*>(synapse::skybox_vert_data);
    const auto* fragData = reinterpret_cast<const char*>(synapse::skybox_frag_data);

    VkShaderModule vertModule = VK_NULL_HANDLE;
    VkShaderModule fragModule = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo moduleInfo{};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = synapse::skybox_vert_size;
    moduleInfo.pCode = reinterpret_cast<const u32*>(vertData);
    SKYBOX_VK_CHECK(vkCreateShaderModule(m_Device, &moduleInfo, nullptr, &vertModule));

    moduleInfo.codeSize = synapse::skybox_frag_size;
    moduleInfo.pCode = reinterpret_cast<const u32*>(fragData);
    SKYBOX_VK_CHECK(vkCreateShaderModule(m_Device, &moduleInfo, nullptr, &fragModule));

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName = "main";

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &blendAttachment;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_SetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    SKYBOX_VK_CHECK(vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_PipelineLayout));

    const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 2;
    createInfo.pStages = stages;
    createInfo.pVertexInputState = nullptr;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisampling;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.pColorBlendState = &colorBlending;
    createInfo.pDynamicState = &dynamicState;
    createInfo.layout = m_PipelineLayout;
    createInfo.renderPass = renderPass;
    createInfo.subpass = 0;
    SKYBOX_VK_CHECK(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr,
                                               &m_Pipeline));

    vkDestroyShaderModule(m_Device, vertModule, nullptr);
    vkDestroyShaderModule(m_Device, fragModule, nullptr);
}

void SkyboxPass::Render(VkCommandBuffer commandBuffer, VkExtent2D extent,
                        const glm::mat4& invViewProj)
{
    VkViewport viewport{};
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1,
                            &m_Set, 0, nullptr);
    vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(invViewProj), &invViewProj);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

} // namespace synapse