#include <renderer/Shadow/ShadowPass.h>

#include <renderer/Pipeline/ShaderCompiler.h>

#include <core/Logger.h>

#include <shadow.vert.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <string>

namespace synapse {
namespace {

#define SHADOW_VK_CHECK(expr)                                                   \
    do                                                                          \
    {                                                                           \
        const VkResult result = (expr);                                         \
        if (result != VK_SUCCESS)                                               \
        {                                                                       \
            SYNAPSE_CORE_CRITICAL("ShadowPass: Vulkan error 0x{:x} at {}:{}",  \
                                  static_cast<u32>(result), __FILE__, __LINE__);\
            std::abort();                                                       \
        }                                                                       \
    } while (false)

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
    SYNAPSE_CORE_CRITICAL("ShadowPass: no suitable memory type");
    std::abort();
}

} // namespace

ShadowPass::ShadowPass(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat depthFormat,
                       u32 size)
    : m_Device(device), m_PhysicalDevice(physicalDevice), m_Format(depthFormat), m_Size(size)
{
    CreateImage();
    CreateSampler();
    CreateRenderPass();
    CreateFramebuffer();

    m_VertSourcePath = SYNAPSE_SHADER_DIR "/shadow.vert.glsl";
    ShaderCompiler::GetModifiedTime(m_VertSourcePath, m_LastVertMtime);
    CreatePipeline(std::string_view(reinterpret_cast<const char*>(synapse::shadow_vert_data),
                                    synapse::shadow_vert_size));

    SYNAPSE_CORE_INFO("Shadow pass created ({}x{} depth map)", m_Size, m_Size);
}

ShadowPass::~ShadowPass()
{
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    vkDestroySampler(m_Device, m_Sampler, nullptr);
    vkDestroyImageView(m_Device, m_ImageView, nullptr);
    vkDestroyImage(m_Device, m_Image, nullptr);
    vkFreeMemory(m_Device, m_Memory, nullptr);
}

void ShadowPass::CreateImage()
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = m_Format;
    imageInfo.extent = {m_Size, m_Size, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    SHADOW_VK_CHECK(vkCreateImage(m_Device, &imageInfo, nullptr, &m_Image));

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(m_Device, m_Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(m_PhysicalDevice, memRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    SHADOW_VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory));
    SHADOW_VK_CHECK(vkBindImageMemory(m_Device, m_Image, m_Memory, 0));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_Format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    SHADOW_VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView));
}

void ShadowPass::CreateSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS;
    samplerInfo.maxLod = 1.0f;
    SHADOW_VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler));
}

void ShadowPass::CreateRenderPass()
{
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_Format;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 0;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &depthAttachment;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;
    SHADOW_VK_CHECK(vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass));
}

void ShadowPass::CreateFramebuffer()
{
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = m_RenderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &m_ImageView;
    createInfo.width = m_Size;
    createInfo.height = m_Size;
    createInfo.layers = 1;
    SHADOW_VK_CHECK(vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Framebuffer));
}

bool ShadowPass::ReloadIfChanged()
{
    const auto now = std::chrono::steady_clock::now();
    if (now - m_LastCheckTime < std::chrono::milliseconds(250))
    {
        return false;
    }
    m_LastCheckTime = now;

    std::filesystem::file_time_type mtime{};
    if (!ShaderCompiler::GetModifiedTime(m_VertSourcePath, mtime))
    {
        return false;
    }
    if (mtime == m_LastVertMtime)
    {
        return false;
    }
    m_LastVertMtime = mtime;

    std::vector<uint8_t> spirv;
    if (!ShaderCompiler::CompileToSpirv(m_VertSourcePath, "vert", spirv))
    {
        return false;
    }

    CreatePipeline(std::string_view(reinterpret_cast<const char*>(spirv.data()), spirv.size()));

    SYNAPSE_CORE_INFO("Shader hot-reload: shadow pipeline reconstruit");
    return true;
}

void ShadowPass::CreatePipeline(std::string_view vertSpirv)
{
    VkShaderModuleCreateInfo moduleInfo{};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = vertSpirv.size();
    moduleInfo.pCode = reinterpret_cast<const u32*>(vertSpirv.data());

    VkShaderModule vertModule = VK_NULL_HANDLE;
    SHADOW_VK_CHECK(vkCreateShaderModule(m_Device, &moduleInfo, nullptr, &vertModule));

    VkPipelineShaderStageCreateInfo stage{};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage.module = vertModule;
    stage.pName = "main";

    VkVertexInputBindingDescription bindings[2]{};
    bindings[0].binding = 0;
    bindings[0].stride = 11 * sizeof(float);
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[1].binding = 1;
    bindings[1].stride = sizeof(glm::mat4);
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    VkVertexInputAttributeDescription attributes[5]{};
    attributes[0].location = 0;
    attributes[0].binding = 0;
    attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[0].offset = 0;
    attributes[1].location = 4;
    attributes[1].binding = 1;
    attributes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[1].offset = 0;
    attributes[2].location = 5;
    attributes[2].binding = 1;
    attributes[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[2].offset = 16;
    attributes[3].location = 6;
    attributes[3].binding = 1;
    attributes[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[3].offset = 32;
    attributes[4].location = 7;
    attributes[4].binding = 1;
    attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[4].offset = 48;

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 2;
    vertexInput.pVertexBindingDescriptions = bindings;
    vertexInput.vertexAttributeDescriptionCount = 5;
    vertexInput.pVertexAttributeDescriptions = attributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.width = static_cast<float>(m_Size);
    viewport.height = static_cast<float>(m_Size);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = {m_Size, m_Size};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = 1.25f;
    rasterizer.depthBiasSlopeFactor = 1.75f;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);

    if (m_PipelineLayout == VK_NULL_HANDLE)
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;
        SHADOW_VK_CHECK(vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_PipelineLayout));
    }

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 1;
    createInfo.pStages = &stage;
    createInfo.pVertexInputState = &vertexInput;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisampling;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.layout = m_PipelineLayout;
    createInfo.renderPass = m_RenderPass;
    createInfo.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    SHADOW_VK_CHECK(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr,
                                               &pipeline));

    vkDestroyShaderModule(m_Device, vertModule, nullptr);

    vkDeviceWaitIdle(m_Device);
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    m_Pipeline = pipeline;
}

void ShadowPass::Render(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer,
                        VkBuffer indexBuffer, VkBuffer instanceBuffer,
                        const std::vector<DrawItem>& items, const glm::mat4& lightViewProj)
{
    VkClearValue clear{};
    clear.depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_Framebuffer;
    renderPassInfo.renderArea.extent = {m_Size, m_Size};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clear;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

    VkDeviceSize offset = 0;
    const VkBuffer buffers[] = {vertexBuffer, instanceBuffer};
    const VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(lightViewProj), &lightViewProj);

    for (const DrawItem& item : items)
    {
        vkCmdDrawIndexed(commandBuffer, item.indexCount, item.instanceCount, item.firstIndex, 0,
                         item.firstInstance);
    }

    vkCmdEndRenderPass(commandBuffer);
}

} // namespace synapse