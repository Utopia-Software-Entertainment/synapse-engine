#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;

layout(set = 0, binding = 1) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(textureSampler, fragUv).rgb * fragColor, 1.0);
}