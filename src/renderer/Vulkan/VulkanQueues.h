#pragma once

namespace synapse {

class VulkanQueues
{
public:
    VulkanQueues() = default;
    ~VulkanQueues() = default;

    VulkanQueues(const VulkanQueues&) = delete;
    VulkanQueues& operator=(const VulkanQueues&) = delete;
};

} // namespace synapse
