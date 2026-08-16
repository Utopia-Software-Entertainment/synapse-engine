#pragma once

#include <core/Types.h>
#include <core/ECS/Registry.h>
#include <glm/glm.hpp>
#include <string>

namespace synapse::physics { class PhysicsWorld; }

namespace synapse {

class Engine;
class Renderer;

class Scene
{
public:
    Scene(Engine& engine, physics::PhysicsWorld& physicsWorld, Renderer& renderer);
    ~Scene() = default;

    ecs::Entity CreateEntity(const std::string& name = "Entity");

    // Loads a glTF model and creates an entity with physics
    ecs::Entity LoadModel(const std::string& filepath,
                          const glm::vec3& position = glm::vec3(0.0f),
                          const glm::vec3& scale = glm::vec3(1.0f),
                          bool isStatic = true);

    void Clear();

private:
    Engine& m_Engine;
    physics::PhysicsWorld& m_PhysicsWorld;
    Renderer& m_Renderer;
};

} // namespace synapse
