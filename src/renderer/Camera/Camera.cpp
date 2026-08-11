#include <renderer/Camera/Camera.h>

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace synapse {

Camera::Camera(f32 fovDegrees, f32 aspectRatio, f32 nearPlane, f32 farPlane)
    : m_FovDegrees(fovDegrees), m_AspectRatio(aspectRatio), m_NearPlane(nearPlane), m_FarPlane(farPlane)
{
}

glm::mat4 Camera::GetViewMatrix() const
{
    const glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), glm::radians(m_Pitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    return rotation * glm::translate(glm::mat4(1.0f), -m_Position);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    glm::mat4 projection =
        glm::perspective(glm::radians(m_FovDegrees), m_AspectRatio, m_NearPlane, m_FarPlane);
    projection[1][1] *= -1.0f; // Y NDC vers le bas en Vulkan
    return projection;
}

void Camera::SetPosition(glm::vec3 position)
{
    m_Position = position;
}

void Camera::SetAspectRatio(f32 aspectRatio)
{
    m_AspectRatio = aspectRatio;
}

void Camera::Rotate(f32 yawDegrees, f32 pitchDegrees)
{
    m_Yaw += yawDegrees;
    m_Pitch = std::clamp(m_Pitch + pitchDegrees, -89.0f, 89.0f);
}

void Camera::Move(glm::vec3 offset)
{
    const f32 yawRad = glm::radians(m_Yaw);
    const f32 pitchRad = glm::radians(m_Pitch);
    const glm::vec3 forward(std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad),
                            std::sin(yawRad) * std::cos(pitchRad));
    const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 up = glm::cross(right, forward);
    m_Position += right * offset.x + up * offset.y + forward * offset.z;
}

} // namespace synapse