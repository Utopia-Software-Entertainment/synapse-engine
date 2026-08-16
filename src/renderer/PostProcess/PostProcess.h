#include <core/Types.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <string_view>
#include <memory>

namespace synapse {

class Pipeline;

class PostProcess
{
public:
    PostProcess(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                VkImageView inputView, VkSampler inputSampler);
    ~PostProcess();

    void Render(VkCommandBuffer commandBuffer, VkExtent2D extent);
    void ReloadIfChanged();

private:
    void CreateDescriptors(VkImageView inputView, VkSampler inputSampler);

    VkDevice m_Device;
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    std::unique_ptr<Pipeline> m_Pipeline;
};

} // namespace synapse
