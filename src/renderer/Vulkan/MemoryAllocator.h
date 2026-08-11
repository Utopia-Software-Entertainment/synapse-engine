#pragma once

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

namespace synapse {

class MemoryAllocator
{
public:
    MemoryAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    ~MemoryAllocator();

    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    VmaAllocator Handle() const { return m_Allocator; }

private:
    VmaAllocator m_Allocator = VK_NULL_HANDLE;
};

} // namespace synapse
