#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 4) in mat4 inInstanceModel;

layout(push_constant) uniform LightVP {
    mat4 lightVP;
} push;

void main()
{
    gl_Position = push.lightVP * inInstanceModel * vec4(inPosition, 1.0);
}
