#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

struct GLFWwindow;

namespace synapse {

class Pipeline;
class Window;

class Renderer
{
public:
    Renderer(Window& window, u32 width, u32 height);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Draw();
    void SetClearColor(glm::vec3 color);
    void SetViewProjection(glm::mat4 view, glm::mat4 proj);

private:
    struct Frame
    {
        VkSemaphore imageAvailable = VK_NULL_HANDLE;
        VkSemaphore renderFinished = VK_NULL_HANDLE;
        VkFence inFlight = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkBuffer uboBuffer = VK_NULL_HANDLE;
        VkDeviceMemory uboMemory = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        void* uboMapped = nullptr;
    };

    struct CameraUBO
    {
        glm::mat4 view;
        glm::mat4 proj;
    };

    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void CreateVertexBuffer();
    void CreateDescriptorObjects();
    void RecreatePipeline();
    void CleanupSwapchain();
    void CleanupDescriptorObjects();
    void RecordCommandBuffer(Frame& frame, u32 imageIndex);

    Window& m_Window;
    u32 m_Width = 0;
    u32 m_Height = 0;
    glm::vec3 m_ClearColor = glm::vec3(0.0f, 0.2f, 0.4f);

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    u32 m_QueueFamilyIndex = 0;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkFormat m_SwapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_SwapchainExtent{};
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::vector<Frame> m_Frames;
    u32 m_FrameIndex = 0;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
    u32 m_VertexCount = 0;

    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    glm::mat4 m_View = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);
};

} // namespace synapse