#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>

namespace synapse {

class SkyboxPass
{
public:
    SkyboxPass(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass,
               u32 queueFamilyIndex, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
    ~SkyboxPass();

    SkyboxPass(const SkyboxPass&) = delete;
    SkyboxPass& operator=(const SkyboxPass&) = delete;

    bool ReloadIfChanged();

    void Render(VkCommandBuffer commandBuffer, VkExtent2D extent,
                const glm::mat4& invViewProj);

private:
    void CreateCubeMap();
    void CreateDescriptors();
    void CreatePipeline(std::string_view vertSpirv, std::string_view fragSpirv,
                        VkRenderPass renderPass);

    VkDevice m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    u32 m_QueueFamilyIndex = 0;
    VkSampleCountFlagBits m_SampleCount = VK_SAMPLE_COUNT_1_BIT;

    static constexpr u32 kFaceSize = 128;
    VkImage m_Image = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    VkImageView m_ImageView = VK_NULL_HANDLE;
    VkSampler m_Sampler = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_SetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_Pool = VK_NULL_HANDLE;
    VkDescriptorSet m_Set = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;

    std::string m_VertSourcePath;
    std::string m_FragSourcePath;
    std::filesystem::file_time_type m_LastVertMtime;
    std::filesystem::file_time_type m_LastFragMtime;
    std::chrono::steady_clock::time_point m_LastCheckTime;
};

} // namespace synapse