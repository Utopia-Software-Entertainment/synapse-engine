#include <core/Engine.h>
#include <core/Types.h>
#include <core/ECS/Components/TransformComponent.h>
#include <core/ECS/Components/MeshComponent.h>
#include <core/Scene.h>
#include <physics/Jolt/PhysicsWorld.h>
#include <physics/Jolt/RigidBody.h>
#include <physics/Jolt/PlayerCharacter.h>
#include <physics/Jolt/PhysicsQuery.h>
#include <physics/Jolt/PhysicsSystem.h>
#include <platform/Window.h>
#include <renderer/Camera/Camera.h>
#include <renderer/Mesh/Mesh.h>
#include <renderer/Mesh/MeshLoader.h>
#include <renderer/Vulkan/Renderer.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <chrono>
#include <cstdio>
#include <cmath>
#include <thread>
#include <vector>

int main()
{
    synapse::Engine engine;
    synapse::Window window("Synapse Engine", 1280, 720);
    synapse::Renderer renderer(window, 1280, 720);

    // --- Start Engine ---
    engine.Start(renderer);
    auto& scene = engine.GetScene();
    auto& world = engine.GetPhysicsWorld(); // CRITICAL: Use Engine's world

    synapse::Camera camera(60.0f, 1280.0f / 720.0f, 0.1f, 2000.0f);
    camera.SetPosition(glm::vec3(0.0f, 2.0f, -10.0f));
    camera.Rotate(180.0f, 0.0f);

    // --- Create a "City" of Monuments (Buildings) ---
    // A dense grid to simulate a plaza/city
    for (int x = -3; x <= 3; ++x) {
        for (int z = -3; z <= 3; ++z) {
            if (x == 0 && z == 0) continue; // Leave center for player
            float posX = x * 40.0f;
            float posZ = z * 40.0f;

            // Load with massive scale, Scene now handles the physical box scale
            scene.LoadModel(SYNAPSE_ASSET_DIR "/models/damaged_helmet.glb",
                            glm::vec3(posX, 0.0f, posZ),
                            glm::vec3(15.0f),
                            true);
        }
    }

    // Manual Floor
    synapse::Mesh floorMesh = synapse::CreateFloor();
    auto floorRange = renderer.UploadMesh(floorMesh);

    synapse::physics::ShapeDesc floorShape;
    floorShape.halfExtent = glm::vec3(1000.0f, 0.5f, 1000.0f);
    synapse::physics::RigidBody floorBody(world, floorShape, glm::vec3(0.0f, -0.5f, 0.0f),
                                          glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                                          synapse::physics::MotionType::Static);

    // Player starts on the ground, sharing the SAME world as buildings
    synapse::physics::PlayerCharacter player(world, glm::vec3(0.0f, 1.0f, -15.0f));

    const glm::vec3 lightDir = glm::normalize(glm::vec3(0.3f, 1.0f, 0.4f));
    const glm::mat4 lightView = glm::lookAt(-lightDir * 100.0f, glm::vec3(0.0f, 0.0f, 0.0f),
                                            glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 lightProj = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 500.0f);
    const glm::mat4 lightViewProj = lightProj * lightView;

    auto startTime = std::chrono::steady_clock::now();
    auto lastTime = startTime;

    GLFWwindow* handle = window.GetHandle();
    glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool firstMouse = true;

    while (engine.Running() && !window.ShouldClose())
    {
        window.PollEvents();

        const float totalTime = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime).count();
        const float deltaTime = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - lastTime).count();
        lastTime = std::chrono::steady_clock::now();

        double mouseX = 0.0;
        double mouseY = 0.0;
        glfwGetCursorPos(handle, &mouseX, &mouseY);
        if (firstMouse)
        {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }
        const float mouseSensitivity = 0.12f;
        camera.Rotate(static_cast<float>(mouseX - lastMouseX) * mouseSensitivity,
                      static_cast<float>(lastMouseY - mouseY) * mouseSensitivity);
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        const float yawRad = glm::radians(camera.GetYaw());
        const glm::vec3 forward(std::sin(yawRad), 0.0f, -std::cos(yawRad));
        const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        glm::vec3 moveDir(0.0f);
        if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) moveDir += forward;
        if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forward;
        if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;
        if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;
        if (glm::length(moveDir) > 0.001f) moveDir = glm::normalize(moveDir);

        const bool jump = glfwGetKey(handle, GLFW_KEY_SPACE) == GLFW_PRESS;

        // --- Step Physics via System (Steps the shared world and synchronizes ECS) ---
        synapse::physics::PhysicsSystem::Update(std::min(deltaTime, 1.0f / 30.0f), world, engine.GetRegistry());

        // Player manual update (CharacterVirtual)
        player.Update(std::min(deltaTime, 1.0f / 30.0f), moveDir, jump);

        const glm::vec3 camPos = player.GetPosition() + glm::vec3(0.0f, 1.6f, 0.0f);
        camera.SetPosition(camPos);
        camera.SetAspectRatio(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight()));

        // --- Prepare Render Data from ECS ---
        std::vector<glm::mat4> transforms;
        std::vector<synapse::DrawItem> items;

        // Add floor
        transforms.push_back(glm::mat4(1.0f));
        items.push_back({floorRange.firstIndex, floorRange.indexCount, 0, 1, floorRange.vertexOffset});

        // Add ECS objects (buildings)
        auto view = engine.GetRegistry().view<synapse::ecs::TransformComponent, synapse::ecs::MeshComponent>();
        for (auto entity : view)
        {
            auto& t = view.get<synapse::ecs::TransformComponent>(entity);
            auto& m = view.get<synapse::ecs::MeshComponent>(entity);
            transforms.push_back(t.LocalMatrix());
            items.push_back({m.firstIndex, m.indexCount, static_cast<uint32_t>(transforms.size() - 1), 1, static_cast<int32_t>(m.vertexOffset)});
        }

        renderer.SetInstanceTransforms(transforms.data(), static_cast<uint32_t>(transforms.size()));
        renderer.SetDrawItems(std::move(items));
        renderer.SetViewProjection(camera.GetViewMatrix(), camera.GetProjectionMatrix());
        renderer.SetLightViewProjection(lightViewProj);
        renderer.Draw(totalTime);
        engine.TickFrame();

        constexpr float kFrameBudget = 1.0f / 60.0f;
        const float frameTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - lastTime).count();
        if (frameTime < kFrameBudget) std::this_thread::sleep_for(std::chrono::duration<float>(kFrameBudget - frameTime));
    }

    return 0;
}
