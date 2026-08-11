#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform LightVP {
    mat4 lightVP;
} push;

void main()
{
    gl_Position = push.lightVP * vec4(inPosition, 1.0);
}
