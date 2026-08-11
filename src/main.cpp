#include <core/Engine.h>
#include <platform/Window.h>
#include <renderer/Camera/Camera.h>
#include <renderer/Vulkan/Renderer.h>

#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>

int main()
{
    synapse::Engine engine;
    synapse::Window window("Synapse Engine", 1280, 720);
    synapse::Renderer renderer(window, 1280, 720);

    synapse::Camera camera(60.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
    camera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));

    engine.Start();

    auto lastTime = std::chrono::steady_clock::now();

    while (engine.Running() && !window.ShouldClose())
    {
        window.PollEvents();

        const float deltaTime = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - lastTime).count();
        lastTime = std::chrono::steady_clock::now();

        const float speed = 3.0f * deltaTime;

        GLFWwindow* handle = window.GetHandle();
        if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.Move(glm::vec3(0.0f, 0.0f, speed));
        }
        if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.Move(glm::vec3(0.0f, 0.0f, -speed));
        }
        if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.Move(glm::vec3(-speed, 0.0f, 0.0f));
        }
        if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.Move(glm::vec3(speed, 0.0f, 0.0f));
        }
        if (glfwGetKey(handle, GLFW_KEY_Q) == GLFW_PRESS)
        {
            camera.Move(glm::vec3(0.0f, -speed, 0.0f));
        }
        if (glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS)
        {
            camera.Move(glm::vec3(0.0f, speed, 0.0f));
        }
        if (glfwGetKey(handle, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(handle, GLFW_KEY_UP) == GLFW_PRESS)
        {
            camera.Rotate(30.0f * deltaTime, 0.0f);
        }
        if (glfwGetKey(handle, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(handle, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            camera.Rotate(-30.0f * deltaTime, 0.0f);
        }

        camera.SetAspectRatio(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight()));
        renderer.SetViewProjection(camera.GetViewMatrix(), camera.GetProjectionMatrix());
        renderer.Draw();
        engine.TickFrame();
    }

    return 0;
}