#pragma once

namespace synapse {

class ImGuiVulkanBackend
{
public:
    ImGuiVulkanBackend() = default;
    ~ImGuiVulkanBackend() = default;

    ImGuiVulkanBackend(const ImGuiVulkanBackend&) = delete;
    ImGuiVulkanBackend& operator=(const ImGuiVulkanBackend&) = delete;
};

} // namespace synapse
