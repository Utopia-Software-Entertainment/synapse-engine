#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform ModelPush {
    mat4 model;
} push;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;

void main()
{
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragUv = inUv;
}