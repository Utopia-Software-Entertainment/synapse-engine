#include <renderer/Pipeline/Pipeline.h>

#include <renderer/Pipeline/ShaderCompiler.h>

#include <core/Logger.h>

#include <cstdlib>

namespace synapse {

Pipeline::Pipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                   const VkDescriptorSetLayout* setLayouts, u32 setLayoutCount,
                   std::string_view vertShader, std::string_view fragShader,
                   u32 pushConstantSize, std::string_view vertSourcePath,
                   std::string_view fragSourcePath,
                   VkSampleCountFlagBits sampleCount)
    : m_Device(device),
      m_RenderPass(renderPass),
      m_Extent(extent),
      m_SetLayouts(setLayouts, setLayouts + setLayoutCount),
      m_PushConstantSize(pushConstantSize),
      m_SampleCount(sampleCount),
      m_VertSourcePath(vertSourcePath),
      m_FragSourcePath(fragSourcePath)
{
    VkShaderModule vertModule = CreateShaderModule(vertShader);
    VkShaderModule fragModule = CreateShaderModule(fragShader);

    m_Pipeline = BuildGraphicsPipeline(vertModule, fragModule);
    if (m_Pipeline == VK_NULL_HANDLE)
    {
        std::abort();
    }

    vkDestroyShaderModule(m_Device, fragModule, nullptr);
    vkDestroyShaderModule(m_Device, vertModule, nullptr);

    if (!m_VertSourcePath.empty())
    {
        ShaderCompiler::GetModifiedTime(m_VertSourcePath, m_LastVertMtime);
    }
    if (!m_FragSourcePath.empty())
    {
        ShaderCompiler::GetModifiedTime(m_FragSourcePath, m_LastFragMtime);
    }

    SYNAPSE_CORE_INFO("Graphics pipeline created");
}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
}

bool Pipeline::ReloadIfChanged()
{
    if (m_VertSourcePath.empty() && m_FragSourcePath.empty())
    {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - m_LastCheckTime < std::chrono::milliseconds(250))
    {
        return false;
    }
    m_LastCheckTime = now;

    bool vertChanged = false;
    bool fragChanged = false;
    std::filesystem::file_time_type vertMtime{};
    std::filesystem::file_time_type fragMtime{};

    if (!m_VertSourcePath.empty() &&
        ShaderCompiler::GetModifiedTime(m_VertSourcePath, vertMtime))
    {
        vertChanged = vertMtime != m_LastVertMtime;
        m_LastVertMtime = vertMtime;
    }
    if (!m_FragSourcePath.empty() &&
        ShaderCompiler::GetModifiedTime(m_FragSourcePath, fragMtime))
    {
        fragChanged = fragMtime != m_LastFragMtime;
        m_LastFragMtime = fragMtime;
    }
    if (!vertChanged && !fragChanged)
    {
        return false;
    }

    std::vector<uint8_t> vertSpirv;
    std::vector<uint8_t> fragSpirv;
    if (!ShaderCompiler::CompileToSpirv(m_VertSourcePath, "vert", vertSpirv))
    {
        return false;
    }
    if (!ShaderCompiler::CompileToSpirv(m_FragSourcePath, "frag", fragSpirv))
    {
        return false;
    }

    VkShaderModule vertModule = CreateShaderModule(
        std::string_view(reinterpret_cast<const char*>(vertSpirv.data()), vertSpirv.size()));
    VkShaderModule fragModule = CreateShaderModule(
        std::string_view(reinterpret_cast<const char*>(fragSpirv.data()), fragSpirv.size()));

    VkPipeline newPipeline = BuildGraphicsPipeline(vertModule, fragModule);
    if (newPipeline == VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_Device, fragModule, nullptr);
        vkDestroyShaderModule(m_Device, vertModule, nullptr);
        return false;
    }

    vkDeviceWaitIdle(m_Device);
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    m_Pipeline = newPipeline;

    vkDestroyShaderModule(m_Device, fragModule, nullptr);
    vkDestroyShaderModule(m_Device, vertModule, nullptr);

    SYNAPSE_CORE_INFO("Shader hot-reload: pipeline principal reconstruit");
    return true;
}

VkPipeline Pipeline::BuildGraphicsPipeline(VkShaderModule vertModule, VkShaderModule fragModule)
{
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName = "main";

    VkVertexInputBindingDescription bindings[2]{};
    bindings[0].binding = 0;
    bindings[0].stride = 11 * sizeof(float);
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[1].binding = 1;
    bindings[1].stride = sizeof(glm::mat4);
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    VkVertexInputAttributeDescription attributes[8]{};
    attributes[0].location = 0;
    attributes[0].binding = 0;
    attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[0].offset = 0;
    attributes[1].location = 1;
    attributes[1].binding = 0;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = 3 * sizeof(float);
    attributes[2].location = 2;
    attributes[2].binding = 0;
    attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[2].offset = 6 * sizeof(float);
    attributes[3].location = 3;
    attributes[3].binding = 0;
    attributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[3].offset = 8 * sizeof(float);
    attributes[4].location = 4;
    attributes[4].binding = 1;
    attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[4].offset = 0;
    attributes[5].location = 5;
    attributes[5].binding = 1;
    attributes[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[5].offset = 16;
    attributes[6].location = 6;
    attributes[6].binding = 1;
    attributes[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[6].offset = 32;
    attributes[7].location = 7;
    attributes[7].binding = 1;
    attributes[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributes[7].offset = 48;

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 2;
    vertexInput.pVertexBindingDescriptions = bindings;
    vertexInput.vertexAttributeDescriptionCount = 8;
    vertexInput.pVertexAttributeDescriptions = attributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_Extent.width);
    viewport.height = static_cast<float>(m_Extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = m_Extent;

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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = m_SampleCount;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

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
    pushConstantRange.size = m_PushConstantSize;

    if (m_Layout == VK_NULL_HANDLE)
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<u32>(m_SetLayouts.size());
        layoutInfo.pSetLayouts = m_SetLayouts.data();
        layoutInfo.pushConstantRangeCount = m_PushConstantSize > 0 ? 1u : 0u;
        layoutInfo.pPushConstantRanges = m_PushConstantSize > 0 ? &pushConstantRange : nullptr;

        if (vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
        {
            SYNAPSE_CORE_CRITICAL("Failed to create pipeline layout");
            return VK_NULL_HANDLE;
        }
    }

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 2;
    createInfo.pStages = stages;
    createInfo.pVertexInputState = &vertexInput;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisampling;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.pColorBlendState = &colorBlending;
    createInfo.layout = m_Layout;
    createInfo.renderPass = m_RenderPass;
    createInfo.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) !=
        VK_SUCCESS)
    {
        SYNAPSE_CORE_CRITICAL("Failed to create graphics pipeline");
        return VK_NULL_HANDLE;
    }
    return pipeline;
}

VkShaderModule Pipeline::CreateShaderModule(std::string_view spirvBytes) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvBytes.size();
    createInfo.pCode = reinterpret_cast<const u32*>(spirvBytes.data());

    VkShaderModule module = VK_NULL_HANDLE;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &module) != VK_SUCCESS)
    {
        SYNAPSE_CORE_CRITICAL("Failed to create shader module");
        std::abort();
    }
    return module;
}

} // namespace synapse
