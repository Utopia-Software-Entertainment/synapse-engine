#include <physics/IK/FABRIKSolver.h>
#include <algorithm>

namespace synapse::physics {

void FABRIKSolver::Solve(std::vector<IKJoint>& joints, const glm::vec3& target, float tolerance, int maxIterations)
{
    if (joints.empty()) return;

    float totalLength = 0.0f;
    for (size_t i = 0; i < joints.size() - 1; ++i)
    {
        totalLength += joints[i].length;
    }

    float distToTarget = glm::distance(joints[0].position, target);

    // Case 1: Target is out of reach
    if (distToTarget > totalLength)
    {
        for (size_t i = 0; i < joints.size() - 1; ++i)
        {
            float r = glm::distance(target, joints[i].position);
            float lambda = joints[i].length / r;
            joints[i + 1].position = (1.0f - lambda) * joints[i].position + lambda * target;
        }
        return;
    }

    // Case 2: Target is reachable
    glm::vec3 basePos = joints[0].position;
    int iteration = 0;
    while (glm::distance(joints.back().position, target) > tolerance && iteration < maxIterations)
    {
        Forward(joints, target);
        Backward(joints, basePos);
        iteration++;
    }
}

void FABRIKSolver::Forward(std::vector<IKJoint>& joints, const glm::vec3& target)
{
    joints.back().position = target;
    for (int i = static_cast<int>(joints.size()) - 2; i >= 0; --i)
    {
        float r = glm::distance(joints[i + 1].position, joints[i].position);
        float lambda = joints[i].length / r;
        joints[i].position = (1.0f - lambda) * joints[i + 1].position + lambda * joints[i].position;
    }
}

void FABRIKSolver::Backward(std::vector<IKJoint>& joints, const glm::vec3& base)
{
    joints[0].position = base;
    for (size_t i = 0; i < joints.size() - 1; ++i)
    {
        float r = glm::distance(joints[i + 1].position, joints[i].position);
        float lambda = joints[i].length / r;
        joints[i + 1].position = (1.0f - lambda) * joints[i].position + lambda * joints[i + 1].position;
    }
}

} // namespace synapse::physics
