#include <core/Types.h>
#include <glm/glm.hpp>
#include <optional>

namespace synapse::physics {

class PhysicsWorld;

struct RaycastHit
{
    glm::vec3 position;
    glm::vec3 normal;
    float distance;
    u32 bodyID;
};

class PhysicsQuery
{
public:
    static std::optional<RaycastHit> RayCast(PhysicsWorld& world, const glm::vec3& origin,
                                             const glm::vec3& direction, float maxDistance);
};

} // namespace synapse::physics
