#include <physics/Jolt/PlayerCharacter.h>

#include <physics/Jolt/PhysicsWorld.h>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace synapse::physics {

namespace {

constexpr float kWalkSpeed = 4.0f;
constexpr float kJumpSpeed = 5.0f;

} // namespace

PlayerCharacter::PlayerCharacter(PhysicsWorld& world, const glm::vec3& position, float radius,
                                 float halfHeight, float mass)
    : m_World(world)
{
    JPH::CharacterVirtualSettings settings;
    settings.mShape = new JPH::CapsuleShape(halfHeight, radius);
    settings.mMass = mass;
    settings.mMaxStrength = 400.0f;
    settings.mCharacterPadding = 0.02f;
    settings.mPredictiveContactDistance = 0.1f;
    settings.mPenetrationRecoverySpeed = 1.0f;

    m_Character = new JPH::CharacterVirtual(
        &settings, JPH::RVec3(position.x, position.y, position.z), JPH::Quat::sIdentity(),
        &m_World.GetPhysicsSystem());
    m_Character->SetListener(this);
}

PlayerCharacter::~PlayerCharacter() = default;

void PlayerCharacter::OnContactAdded(const JPH::CharacterVirtual* inCharacter,
                                     const JPH::BodyID& inBodyID2,
                                     const JPH::SubShapeID& inSubShapeID2,
                                     JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal,
                                     JPH::CharacterContactSettings& ioSettings)
{
    // Si on touche un objet dynamique, on lui applique une force pour le "pousser".
    if (m_World.GetBodyInterface().GetMotionType(inBodyID2) == JPH::EMotionType::Dynamic)
    {
        // On calcule l'impulsion basée sur la vitesse du personnage.
        JPH::Vec3 velocity = inCharacter->GetLinearVelocity();
        float speed = velocity.Dot(inContactNormal);

        // Si on pousse l'objet (vitesse positive vers l'objet)
        if (speed > 0.01f)
        {
            // Impulsion proportionnelle à la vitesse et à la masse du personnage.
            // On utilise la masse du personnage (default 70kg) pour donner du poids à l'impact.
            m_World.GetBodyInterface().AddImpulse(inBodyID2, inContactNormal * speed * inCharacter->GetMass());
        }
    }
}

void PlayerCharacter::Update(float deltaTime, const glm::vec3& moveDirection, bool jump)
{
    const JPH::Vec3 gravity = JPH::Vec3(m_World.GetGravity().x, m_World.GetGravity().y, m_World.GetGravity().z);
    JPH::Vec3 velocity = m_Character->GetLinearVelocity();

    if (m_Character->IsSupported())
    {
        // Au sol : suit l'input horizontal et la vitesse du sol (plateformes).
        JPH::Vec3 horizontal(moveDirection.x * kWalkSpeed, 0.0f, moveDirection.z * kWalkSpeed);
        velocity = horizontal + m_Character->GetGroundVelocity();

        if (jump)
        {
            velocity.SetY(kJumpSpeed);
        }
        else
        {
            // On applique une petite force vers le bas pour rester collé au sol/pentes,
            // équivalente à un pas de gravité, sans accumulation.
            velocity.SetY(gravity.GetY() * deltaTime);
        }
    }
    else
    {
        // En l'air : on applique la gravité et on permet un contrôle horizontal.
        velocity.SetX(moveDirection.x * kWalkSpeed);
        velocity.SetZ(moveDirection.z * kWalkSpeed);
        velocity += gravity * deltaTime;
    }

    m_Character->SetLinearVelocity(velocity);

    const JPH::DefaultBroadPhaseLayerFilter broadPhaseFilter =
        m_World.GetPhysicsSystem().GetDefaultBroadPhaseLayerFilter(
            static_cast<JPH::ObjectLayer>(ObjectLayer::Dynamic));
    const JPH::DefaultObjectLayerFilter objectLayerFilter =
        m_World.GetPhysicsSystem().GetDefaultLayerFilter(
            static_cast<JPH::ObjectLayer>(ObjectLayer::Dynamic));
    const JPH::BodyFilter bodyFilter;
    const JPH::ShapeFilter shapeFilter;

    // On passe un vecteur gravité nul à ExtendedUpdate car on a déjà intégré la gravité
    // dans la vitesse au-dessus. Cela évite l'effet "double gravité" tout en conservant
    // un mouvement fluide et prévisible.
    m_Character->ExtendedUpdate(deltaTime, JPH::Vec3::sZero(),
                                JPH::CharacterVirtual::ExtendedUpdateSettings{},
                                broadPhaseFilter, objectLayerFilter, bodyFilter, shapeFilter,
                                m_World.GetTempAllocator());
}

glm::vec3 PlayerCharacter::GetPosition() const
{
    const JPH::RVec3 pos = m_Character->GetPosition();
    return glm::vec3(static_cast<float>(pos.GetX()), static_cast<float>(pos.GetY()),
                     static_cast<float>(pos.GetZ()));
}

glm::mat4 PlayerCharacter::GetModelMatrix() const
{
    return glm::translate(glm::mat4(1.0f), GetPosition());
}

bool PlayerCharacter::IsOnGround() const
{
    return m_Character->IsSupported();
}

glm::vec3 PlayerCharacter::GetLinearVelocity() const
{
    const JPH::Vec3 v = m_Character->GetLinearVelocity();
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

} // namespace synapse::physics
