#pragma once

#include <core/Types.h>

#include <functional>
#include <string>

struct GLFWwindow;

namespace synapse {

class Window
{
public:
    using ResizeCallback = std::function<void(u32 width, u32 height)>;

    Window(std::string title, u32 width, u32 height);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void PollEvents();
    bool ShouldClose() const;

    u32 GetWidth() const { return m_Width; }
    u32 GetHeight() const { return m_Height; }
    float GetAspectRatio() const
    {
        return m_Height == 0 ? 1.0f : static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }
    GLFWwindow* GetHandle() const { return m_Handle; }

    // Fired when the framebuffer size changes (resize → swapchain rebuild).
    void SetResizeCallback(ResizeCallback callback);

private:
    std::string m_Title;
    u32 m_Width = 0;
    u32 m_Height = 0;
    GLFWwindow* m_Handle = nullptr;
    ResizeCallback m_ResizeCallback;
};

} // namespace synapse