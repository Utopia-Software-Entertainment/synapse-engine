#include <core/Types.h>
#include <glm/glm.hpp>
#include <vector>

namespace synapse::physics {

struct IKJoint
{
    glm::vec3 position;
    float length;
};

class FABRIKSolver
{
public:
    static void Solve(std::vector<IKJoint>& joints, const glm::vec3& target, float tolerance = 0.01f, int maxIterations = 10);

private:
    static void Forward(std::vector<IKJoint>& joints, const glm::vec3& target);
    static void Backward(std::vector<IKJoint>& joints, const glm::vec3& base);
};

} // namespace synapse::physics
