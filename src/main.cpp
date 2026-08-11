#include <core/Engine.h>
#include <core/Types.h>
#include <platform/Window.h>
#include <renderer/Camera/Camera.h>
#include <renderer/Mesh/Mesh.h>
#include <renderer/Mesh/MeshLoader.h>
#include <renderer/Vulkan/Renderer.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <cmath>
#include <vector>

int main()
{
    synapse::Engine engine;
    synapse::Window window("Synapse Engine", 1280, 720);
    synapse::Renderer renderer(window, 1280, 720);

    synapse::Camera camera(60.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
    camera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));

    synapse::Mesh cube = synapse::CreateCube();
    synapse::Mesh floor = synapse::CreateFloor();
    synapse::Mesh sphere = synapse::MeshLoader::LoadObj("assets/models/sphere.obj");

    synapse::Mesh scene;
    scene.vertices = cube.vertices;
    scene.vertices.insert(scene.vertices.end(), floor.vertices.begin(), floor.vertices.end());
    scene.vertices.insert(scene.vertices.end(), sphere.vertices.begin(), sphere.vertices.end());

    constexpr uint32_t kCubeVertexOffset = 0;
    const uint32_t kFloorVertexOffset = static_cast<uint32_t>(cube.vertices.size());
    const uint32_t kSphereVertexOffset = kFloorVertexOffset + static_cast<uint32_t>(floor.vertices.size());

    scene.indices = cube.indices;
    for (uint16_t index : floor.indices)
    {
        scene.indices.push_back(index + kFloorVertexOffset);
    }
    for (uint16_t index : sphere.indices)
    {
        scene.indices.push_back(index + kSphereVertexOffset);
    }

    renderer.SetGeometry(scene);

    constexpr uint32_t kCubeIndexCount = 36;
    constexpr uint32_t kCubeFirstIndex = 0;
    const uint32_t kFloorFirstIndex = static_cast<uint32_t>(cube.indices.size());
    constexpr uint32_t kFloorIndexCount = 6;
    const uint32_t kSphereFirstIndex = kFloorFirstIndex + kFloorIndexCount;
    const uint32_t kSphereIndexCount = static_cast<uint32_t>(sphere.indices.size());

    engine.Start();

    const glm::vec3 lightDir = glm::normalize(glm::vec3(0.3f, 1.0f, 0.4f));
    const glm::mat4 lightView = glm::lookAt(-lightDir * 8.0f, glm::vec3(0.0f, 0.0f, 0.0f),
                                            glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 lightProj = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 30.0f);
    const glm::mat4 lightViewProj = lightProj * lightView;

    auto lastTime = std::chrono::steady_clock::now();
    const auto startTime = lastTime;

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

        const float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime).count();

        constexpr uint32_t kCubeIndexCount = 36;
        constexpr uint32_t kCubeFirstIndex = 0;
        constexpr uint32_t kFloorFirstIndex = 36;
        constexpr uint32_t kFloorIndexCount = 6;

        std::vector<synapse::DrawItem> items;
        items.reserve(9);

        items.push_back({glm::mat4(1.0f), kFloorFirstIndex, kFloorIndexCount});

        items.push_back({glm::rotate(glm::mat4(1.0f), elapsed * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)),
                         kCubeFirstIndex, kCubeIndexCount});

        items.push_back({glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.3f, 0.0f)) *
                             glm::rotate(glm::mat4(1.0f), elapsed * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f)),
                         kSphereFirstIndex, kSphereIndexCount});

        for (uint32_t i = 0; i < 6; ++i)
        {
            const float angle = elapsed * 0.9f + static_cast<float>(i) * glm::two_pi<float>() / 6.0f;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(
                std::cos(angle) * 2.2f, 0.4f, std::sin(angle) * 2.2f));
            model = model * glm::rotate(glm::mat4(1.0f), elapsed * 1.5f, glm::vec3(0.0f, 1.0f, 0.0f));
            items.push_back({model, kCubeFirstIndex, kCubeIndexCount});
        }

        renderer.SetDrawItems(std::move(items));
        renderer.SetViewProjection(camera.GetViewMatrix(), camera.GetProjectionMatrix());
        renderer.SetLightViewProjection(lightViewProj);
        renderer.Draw();
        engine.TickFrame();
    }

    return 0;
}