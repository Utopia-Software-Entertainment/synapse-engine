#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>

namespace synapse {

class Camera
{
public:
    Camera(f32 fovDegrees, f32 aspectRatio, f32 nearPlane, f32 farPlane);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    void SetPosition(glm::vec3 position);
    glm::vec3 GetPosition() const { return m_Position; }

    void SetAspectRatio(f32 aspectRatio);
    void Rotate(f32 yawDegrees, f32 pitchDegrees);
    void Move(glm::vec3 offset);

    f32 GetYaw() const { return m_Yaw; }
    f32 GetPitch() const { return m_Pitch; }

private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    f32 m_Yaw = -90.0f;
    f32 m_Pitch = 0.0f;
    f32 m_FovDegrees = 60.0f;
    f32 m_AspectRatio = 1.0f;
    f32 m_NearPlane = 0.1f;
    f32 m_FarPlane = 100.0f;
};

} // namespace synapse