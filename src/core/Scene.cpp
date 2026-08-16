#include <core/Scene.h>
#include <core/Engine.h>
#include <core/Logger.h>
#include <core/ECS/Components/TransformComponent.h>
#include <core/ECS/Components/MeshComponent.h>
#include <core/ECS/Components/PhysicsBodyComponent.h>
#include <renderer/Mesh/MeshLoader.h>
#include <physics/Jolt/RigidBody.h>
#include <physics/Jolt/PhysicsWorld.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include <algorithm>
#include <limits>

#include <renderer/Vulkan/Renderer.h>

#include <Jolt/Jolt.h>

namespace synapse {

Scene::Scene(Engine& engine, physics::PhysicsWorld& physicsWorld, Renderer& renderer)
    : m_Engine(engine), m_PhysicsWorld(physicsWorld), m_Renderer(renderer)
{
}

ecs::Entity Scene::CreateEntity(const std::string& name)
{
    ecs::Entity entity = m_Engine.GetRegistry().create();
    ecs::AddComponent<ecs::TransformComponent>(m_Engine.GetRegistry(), entity);
    return entity;
}

ecs::Entity Scene::LoadModel(const std::string& filepath, const glm::vec3& position, const glm::vec3& scale, bool isStatic)
{
    ecs::Registry& registry = m_Engine.GetRegistry();
    ecs::Entity entity = CreateEntity(filepath);

    auto& transform = registry.get<ecs::TransformComponent>(entity);
    transform.position = position;
    transform.scale = scale;

    // 1. Load the mesh
    Mesh modelMesh = MeshLoader::LoadGltf(filepath);
    if (modelMesh.vertices.empty())
    {
        // Try fallback to Obj if gltf failed or empty
        modelMesh = MeshLoader::LoadObj(filepath);
        if (modelMesh.vertices.empty()) return entity;
    }

    auto& meshComp = registry.emplace<ecs::MeshComponent>(entity);
    meshComp.name = filepath;

    // 2. Upload to GPU
    Renderer::MeshRange range = m_Renderer.UploadMesh(modelMesh);
    meshComp.firstIndex = range.firstIndex;
    meshComp.indexCount = range.indexCount;
    meshComp.vertexOffset = range.vertexOffset;

    // 3. Calculate Bounding Box for basic physics
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::lowest());

    for (const auto& v : modelMesh.vertices)
    {
        min = glm::min(min, v.position);
        max = glm::max(max, v.position);
    }

    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 halfExtent = (max - min) * 0.5f;

    // Apply scale to center and halfExtent
    center *= scale;
    halfExtent *= scale;

    // 4. Create Physics Body
    physics::ShapeDesc desc;
    desc.kind = physics::ShapeKind::Box;
    desc.halfExtent = halfExtent;

    // We keep a pointer to a manually allocated RigidBody for now.
    auto* rigidBody = new physics::RigidBody(
        m_PhysicsWorld, desc, position + center,
        glm::quat(1,0,0,0),
        isStatic ? physics::MotionType::Static : physics::MotionType::Dynamic
    );

    auto& physComp = registry.emplace<ecs::PhysicsBodyComponent>(entity);
    physComp.bodyIDValue = rigidBody->GetBodyID();
    physComp.type = isStatic ? ecs::BodyType::Static : ecs::BodyType::Dynamic;
    physComp.offset = center;

    return entity;
}

void Scene::Clear()
{
    m_Engine.GetRegistry().clear();
}

} // namespace synapse
