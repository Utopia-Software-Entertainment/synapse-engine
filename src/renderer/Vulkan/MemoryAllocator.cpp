#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <renderer/Vulkan/MemoryAllocator.h>

namespace synapse {

MemoryAllocator::MemoryAllocator(VkInstance instance, VkPhysicalDevice physicalDevice,
                                 VkDevice device)
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    if (vmaCreateAllocator(&allocatorInfo, &m_Allocator) != VK_SUCCESS)
    {
        std::abort();
    }
}

MemoryAllocator::~MemoryAllocator()
{
    if (m_Allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_Allocator);
    }
}

} // namespace synapse
