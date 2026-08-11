#pragma once

namespace synapse {

class VulkanDevice
{
public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
};

} // namespace synapse
