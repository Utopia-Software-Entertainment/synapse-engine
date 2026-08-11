#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace synapse {

class Pipeline
{
public:
    Pipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
             const VkDescriptorSetLayout* setLayouts, u32 setLayoutCount,
             std::string_view vertShader, std::string_view fragShader,
             u32 pushConstantSize = 0, std::string_view vertSourcePath = {},
             std::string_view fragSourcePath = {},
             VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    VkPipeline GetHandle() const { return m_Pipeline; }
    VkPipelineLayout GetLayout() const { return m_Layout; }

    bool ReloadIfChanged();

private:
    VkShaderModule CreateShaderModule(std::string_view spirvBytes) const;
    VkPipeline BuildGraphicsPipeline(VkShaderModule vertModule, VkShaderModule fragModule);

    VkDevice m_Device = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkExtent2D m_Extent{};
    std::vector<VkDescriptorSetLayout> m_SetLayouts;
    u32 m_PushConstantSize = 0;
    VkSampleCountFlagBits m_SampleCount = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

    std::string m_VertSourcePath;
    std::string m_FragSourcePath;
    std::filesystem::file_time_type m_LastVertMtime;
    std::filesystem::file_time_type m_LastFragMtime;
    std::chrono::steady_clock::time_point m_LastCheckTime;
};

} // namespace synapse
