#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>
#include <memory>

namespace JPH {

class PhysicsSystem;
class JobSystem;
class TempAllocator;
class BroadPhaseLayerInterfaceTable;
class ObjectVsBroadPhaseLayerFilterTable;
class ObjectLayerPairFilterTable;
class BodyInterface;

} // namespace JPH

namespace synapse::physics {

enum class ObjectLayer
{
    Static = 0,
    Dynamic = 1,
};

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    void Step(f32 deltaTime, int collisionSteps = 1);

    JPH::BodyInterface& GetBodyInterface();
    JPH::PhysicsSystem& GetPhysicsSystem();
    JPH::TempAllocator& GetTempAllocator();
    glm::vec3 GetGravity() const;

private:
    // Ordre de destruction important : PhysicsSystem (déclaré en dernier) doit
    // être détruit AVANT le JobSystem et le TempAllocator.
    std::unique_ptr<JPH::JobSystem> m_JobSystem;
    std::unique_ptr<JPH::TempAllocator> m_TempAllocator;
    std::unique_ptr<JPH::BroadPhaseLayerInterfaceTable> m_BroadPhaseLayers;
    std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> m_BroadPhaseFilter;
    std::unique_ptr<JPH::ObjectLayerPairFilterTable> m_ObjectFilter;
    std::unique_ptr<JPH::PhysicsSystem> m_System;
};

} // namespace synapse::physics
