#pragma once

namespace synapse {

class VulkanContext
{
public:
    VulkanContext() = default;
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
};

} // namespace synapse
