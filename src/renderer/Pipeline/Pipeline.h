#pragma once

#include <core/Types.h>

#include <vulkan/vulkan.h>

#include <string>

namespace synapse {

class Pipeline
{
public:
    Pipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
             std::string_view vertShader, std::string_view fragShader);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    VkPipeline GetHandle() const { return m_Pipeline; }

private:
    VkShaderModule CreateShaderModule(std::string_view spirvBytes) const;

    VkDevice m_Device = VK_NULL_HANDLE;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
};

} // namespace synapse