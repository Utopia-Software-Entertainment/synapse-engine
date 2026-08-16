#include <physics/Jolt/PhysicsWorld.h>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <thread>

namespace synapse::physics {

namespace {

bool EnsureJoltInitialized()
{
    static bool initialized = false;
    if (!initialized)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        initialized = true;
    }
    return initialized;
}

} // namespace

PhysicsWorld::PhysicsWorld()
{
    EnsureJoltInitialized();

    const int hwThreads = static_cast<int>(std::thread::hardware_concurrency());
    const int kNumThreads = hwThreads > 1 ? hwThreads - 1 : 1;
    m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(1024, 1024, kNumThreads);
    m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(16 * 1024 * 1024);

    m_BroadPhaseLayers = std::make_unique<JPH::BroadPhaseLayerInterfaceTable>(
        2, static_cast<JPH::uint>(ObjectLayer::Dynamic) + 1);
    m_BroadPhaseLayers->MapObjectToBroadPhaseLayer(
        static_cast<JPH::ObjectLayer>(ObjectLayer::Static), JPH::BroadPhaseLayer(0));
    m_BroadPhaseLayers->MapObjectToBroadPhaseLayer(
        static_cast<JPH::ObjectLayer>(ObjectLayer::Dynamic), JPH::BroadPhaseLayer(1));

    m_ObjectFilter = std::make_unique<JPH::ObjectLayerPairFilterTable>(
        static_cast<JPH::uint>(ObjectLayer::Dynamic) + 1);
    m_ObjectFilter->EnableCollision(static_cast<JPH::ObjectLayer>(ObjectLayer::Static),
                                    static_cast<JPH::ObjectLayer>(ObjectLayer::Static));
    m_ObjectFilter->EnableCollision(static_cast<JPH::ObjectLayer>(ObjectLayer::Static),
                                    static_cast<JPH::ObjectLayer>(ObjectLayer::Dynamic));
    m_ObjectFilter->EnableCollision(static_cast<JPH::ObjectLayer>(ObjectLayer::Dynamic),
                                    static_cast<JPH::ObjectLayer>(ObjectLayer::Dynamic));

    m_BroadPhaseFilter = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
        *m_BroadPhaseLayers, 2, *m_ObjectFilter,
        static_cast<JPH::uint>(ObjectLayer::Dynamic) + 1);

    m_System = std::make_unique<JPH::PhysicsSystem>();
    m_System->Init(4096, 8, 65536, 16384, *m_BroadPhaseLayers, *m_BroadPhaseFilter,
                   *m_ObjectFilter);
    m_System->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
}

PhysicsWorld::~PhysicsWorld() = default;

void PhysicsWorld::Step(f32 deltaTime, int collisionSteps)
{
    m_System->Update(deltaTime, collisionSteps, m_TempAllocator.get(), m_JobSystem.get());
}

JPH::BodyInterface& PhysicsWorld::GetBodyInterface()
{
    return m_System->GetBodyInterface();
}

JPH::PhysicsSystem& PhysicsWorld::GetPhysicsSystem()
{
    return *m_System;
}

JPH::TempAllocator& PhysicsWorld::GetTempAllocator()
{
    return *m_TempAllocator;
}

glm::vec3 PhysicsWorld::GetGravity() const
{
    const JPH::Vec3 g = m_System->GetGravity();
    return glm::vec3(g.GetX(), g.GetY(), g.GetZ());
}

} // namespace synapse::physics
