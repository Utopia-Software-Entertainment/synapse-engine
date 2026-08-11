#pragma once

#include <renderer/Vulkan/DrawItem.h>

#include <core/Types.h>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace synapse {

class ShadowPass
{
public:
    ShadowPass(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat depthFormat,
               u32 size = 2048);
    ~ShadowPass();

    ShadowPass(const ShadowPass&) = delete;
    ShadowPass& operator=(const ShadowPass&) = delete;

    VkImageView GetImageView() const { return m_ImageView; }
    VkSampler GetSampler() const { return m_Sampler; }

    bool ReloadIfChanged();

    void Render(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer,
                VkBuffer instanceBuffer, const std::vector<DrawItem>& items,
                const glm::mat4& lightViewProj);

private:
    void CreateImage();
    void CreateSampler();
    void CreateRenderPass();
    void CreateFramebuffer();
    void CreatePipeline(std::string_view vertSpirv);

    VkDevice m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkFormat m_Format = VK_FORMAT_UNDEFINED;
    u32 m_Size = 2048;

    VkImage m_Image = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    VkImageView m_ImageView = VK_NULL_HANDLE;
    VkSampler m_Sampler = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

    std::string m_VertSourcePath;
    std::filesystem::file_time_type m_LastVertMtime;
    std::chrono::steady_clock::time_point m_LastCheckTime;
};

} // namespace synapse