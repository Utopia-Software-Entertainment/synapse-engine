#include <platform/Window.h>

#include <core/Logger.h>

#include <GLFW/glfw3.h>

#include <algorithm>

namespace synapse {

Window::Window(std::string title, u32 width, u32 height)
    : m_Title(std::move(title)), m_Width(width), m_Height(height)
{
    if (glfwInit() != GLFW_TRUE)
    {
        SYNAPSE_CORE_CRITICAL("GLFW initialization failed");
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_Handle = glfwCreateWindow(static_cast<int>(m_Width), static_cast<int>(m_Height),
                                m_Title.c_str(), nullptr, nullptr);
    if (m_Handle == nullptr)
    {
        SYNAPSE_CORE_CRITICAL("GLFW failed to create window");
        glfwTerminate();
        m_Handle = nullptr;
        return;
    }

    glfwSetWindowUserPointer(m_Handle, this);

    SYNAPSE_CORE_INFO("Window created: {} ({}x{})", m_Title, m_Width, m_Height);
}

Window::~Window()
{
    if (m_Handle != nullptr)
    {
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr;
    }
    glfwTerminate();
}

void Window::SetResizeCallback(ResizeCallback callback)
{
    m_ResizeCallback = std::move(callback);
}

void Window::PollEvents()
{
    glfwPollEvents();

    if (m_Handle != nullptr)
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(m_Handle, &width, &height);
        const u32 newWidth = static_cast<u32>(std::max(width, 1));
        const u32 newHeight = static_cast<u32>(std::max(height, 1));

        if (newWidth != m_Width || newHeight != m_Height)
        {
            m_Width = newWidth;
            m_Height = newHeight;
            if (m_ResizeCallback)
            {
                m_ResizeCallback(m_Width, m_Height);
            }
        }
    }
}

bool Window::ShouldClose() const
{
    return m_Handle == nullptr || glfwWindowShouldClose(m_Handle) == GLFW_TRUE;
}

} // namespace synapse