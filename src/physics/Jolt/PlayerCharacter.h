#pragma once

#include <core/Types.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace JPH {

class CharacterVirtual;

} // namespace JPH

namespace synapse::physics {

class PhysicsWorld;

// Contrôleur de personnage (capsule) basé sur JPH::CharacterVirtual.
// Le déplacement se fait par balayage de la forme : le personnage NE PEUT PAS
// traverser les objets statiques ni les corps dynamiques.
class PlayerCharacter : public JPH::CharacterContactListener
{
public:
    PlayerCharacter(PhysicsWorld& world, const glm::vec3& position, float radius = 0.4f,
                    float halfHeight = 0.8f, float mass = 70.0f);
    virtual ~PlayerCharacter() override;

    PlayerCharacter(const PlayerCharacter&) = delete;
    PlayerCharacter& operator=(const PlayerCharacter&) = delete;

    // --- CharacterContactListener interface ---
    virtual void OnContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2,
                                const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition,
                                JPH::Vec3Arg inContactNormal,
                                JPH::CharacterContactSettings& ioSettings) override;

    // moveDirection : direction horizontale (XZ) en espace monde, normalisée.
    void Update(float deltaTime, const glm::vec3& moveDirection, bool jump);

    glm::vec3 GetPosition() const;
    glm::mat4 GetModelMatrix() const;
    bool IsOnGround() const;
    glm::vec3 GetLinearVelocity() const;

private:
    PhysicsWorld& m_World;
    JPH::Ref<JPH::CharacterVirtual> m_Character;
};

} // namespace synapse::physics