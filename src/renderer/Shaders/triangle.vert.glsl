#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in mat4 inInstanceModel;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 lightVP;
} ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec4 fragLightPos;

void main()
{
    vec4 worldPos = inInstanceModel * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPos;
    fragColor = inColor;
    fragUv = inUv;
    fragNormal = mat3(inInstanceModel) * inNormal;
    fragLightPos = ubo.lightVP * worldPos;
}
