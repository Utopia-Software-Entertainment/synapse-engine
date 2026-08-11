#include <platform/Window.h>

#include <core/Logger.h>

#include <GLFW/glfw3.h>

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
    m_Handle = glfwCreateWindow(static_cast<int>(m_Width), static_cast<int>(m_Height),
                                m_Title.c_str(), nullptr, nullptr);
    if (m_Handle == nullptr)
    {
        SYNAPSE_CORE_CRITICAL("GLFW failed to create window");
        glfwTerminate();
        return;
    }

    SYNAPSE_CORE_INFO("Window created: {} ({}x{})", m_Title, m_Width, m_Height);
}

Window::~Window()
{
    if (m_Handle != nullptr)
    {
        glfwDestroyWindow(m_Handle);
    }
    glfwTerminate();
}

void Window::PollEvents()
{
    glfwPollEvents();
}

bool Window::ShouldClose() const
{
    return m_Handle == nullptr || glfwWindowShouldClose(m_Handle) == GLFW_TRUE;
}

} // namespace synapse