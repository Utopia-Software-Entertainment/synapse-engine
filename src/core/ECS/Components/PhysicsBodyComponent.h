#pragma once

#include <core/Types.h>
#include <glm/glm.hpp>
#pragma once

#include <core/Types.h>
#include <glm/glm.hpp>

namespace synapse::ecs {

enum class BodyType
{
    Static,
    Dynamic,
    Kinematic
};

struct PhysicsBodyComponent
{
    u32 bodyIDValue = 0; // Raw value of JPH::BodyID
    BodyType type = BodyType::Static;

    // Offset relative to the entity transform
    glm::vec3 offset = glm::vec3(0.0f);
};

} // namespace synapse::ecs
