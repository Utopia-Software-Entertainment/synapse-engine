#include <core/Engine.h>
#include <core/Types.h>
#include <physics/Jolt/PhysicsWorld.h>
#include <physics/Jolt/RigidBody.h>
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

    synapse::physics::PhysicsWorld world;

    synapse::physics::ShapeDesc floorShape;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    synapse::physics::RigidBody floorBody(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                                          glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                                          synapse::physics::MotionType::Static);

    synapse::physics::RigidBody cubeBody(world, synapse::physics::ShapeDesc{},
                                         glm::vec3(0.0f, 0.5f, 0.0f));

    synapse::physics::ShapeDesc sphereShape;
    sphereShape.kind = synapse::physics::ShapeKind::Sphere;
    sphereShape.radius = 0.5f;
    synapse::physics::RigidBody sphereBody(world, sphereShape, glm::vec3(-1.5f, 2.0f, 0.0f),
                                           glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                                           synapse::physics::MotionType::Dynamic, 0.5f, 0.35f);
    sphereBody.SetLinearVelocity(glm::vec3(2.2f, 0.0f, 0.0f));

    std::array<synapse::physics::RigidBody, 6> orbitBodies{
        synapse::physics::RigidBody(world, synapse::physics::ShapeDesc{},
                                    glm::vec3(std::cos(0.0f) * 2.2f, 2.0f, std::sin(0.0f) * 2.2f)),
        synapse::physics::RigidBody(world, synapse::physics::ShapeDesc{},
                                    glm::vec3(std::cos(glm::two_pi<float>() / 6.0f) * 2.2f, 2.5f,
                                              std::sin(glm::two_pi<float>() / 6.0f) * 2.2f)),
        synapse::physics::RigidBody(world, synapse::physics::ShapeDesc{},
                                    glm::vec3(std::cos(2.0f * glm::two_pi<float>() / 6.0f) * 2.2f, 3.0f,
                                              std::sin(2.0f * glm::two_pi<float>() / 6.0f) * 2.2f)),
        synapse::physics::RigidBody(world, synapse::physics::ShapeDesc{},
                                    glm::vec3(std::cos(3.0f * glm::two_pi<float>() / 6.0f) * 2.2f, 3.5f,
                                              std::sin(3.0f * glm::two_pi<float>() / 6.0f) * 2.2f)),
        synapse::physics::RigidBody(world, synapse::physics::ShapeDesc{},
                                    glm::vec3(std::cos(4.0f * glm::two_pi<float>() / 6.0f) * 2.2f, 4.0f,
                                              std::sin(4.0f * glm::two_pi<float>() / 6.0f) * 2.2f)),
        synapse::physics::RigidBody(world, synapse::physics::ShapeDesc{},
                                    glm::vec3(std::cos(5.0f * glm::two_pi<float>() / 6.0f) * 2.2f, 4.5f,
                                              std::sin(5.0f * glm::two_pi<float>() / 6.0f) * 2.2f)),
    };
    for (uint32_t i = 0; i < 6; ++i)
    {
        const float angle = static_cast<float>(i) * glm::two_pi<float>() / 6.0f;
        orbitBodies[i].SetLinearVelocity(
            glm::vec3(-std::sin(angle) * 1.5f, 0.0f, std::cos(angle) * 1.5f));
    }

    engine.Start();

    const glm::vec3 lightDir = glm::normalize(glm::vec3(0.3f, 1.0f, 0.4f));
    const glm::mat4 lightView = glm::lookAt(-lightDir * 8.0f, glm::vec3(0.0f, 0.0f, 0.0f),
                                            glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 lightProj = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 30.0f);
    const glm::mat4 lightViewProj = lightProj * lightView;

    auto lastTime = std::chrono::steady_clock::now();

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

        const float deltaTime = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - lastTime).count();
        lastTime = std::chrono::steady_clock::now();

        const float speed = 3.0f * deltaTime;
        const float lookSpeed = 30.0f * deltaTime;

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
        if (glfwGetKey(handle, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            camera.Rotate(-lookSpeed, 0.0f);
        }
        if (glfwGetKey(handle, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            camera.Rotate(lookSpeed, 0.0f);
        }
        if (glfwGetKey(handle, GLFW_KEY_UP) == GLFW_PRESS)
        {
            camera.Rotate(0.0f, lookSpeed);
        }
        if (glfwGetKey(handle, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            camera.Rotate(0.0f, -lookSpeed);
        }

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

        camera.SetAspectRatio(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight()));

        world.Step(std::min(deltaTime, 1.0f / 30.0f));

        std::vector<glm::mat4> transforms;
        std::vector<synapse::DrawItem> items;
        transforms.reserve(9);
        items.reserve(4);

        transforms.push_back(glm::mat4(1.0f));
        items.push_back({kFloorFirstIndex, kFloorIndexCount, 0, 1});

        transforms.push_back(cubeBody.GetModelMatrix());
        items.push_back({kCubeFirstIndex, kCubeIndexCount, 1, 1});

        transforms.push_back(sphereBody.GetModelMatrix());
        items.push_back({kSphereFirstIndex, kSphereIndexCount, 2, 1});

        for (const auto& body : orbitBodies)
        {
            transforms.push_back(body.GetModelMatrix());
        }
        items.push_back({kCubeFirstIndex, kCubeIndexCount, 3, 6});

        renderer.SetInstanceTransforms(transforms.data(), static_cast<uint32_t>(transforms.size()));
        renderer.SetDrawItems(std::move(items));
        renderer.SetViewProjection(camera.GetViewMatrix(), camera.GetProjectionMatrix());
        renderer.SetLightViewProjection(lightViewProj);
        renderer.Draw();
        engine.TickFrame();
    }

    return 0;
}