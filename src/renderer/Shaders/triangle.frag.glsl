#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;

layout(set = 0, binding = 1) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.4));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);
    float lighting = 0.15 + 0.85 * diffuse;
    outColor = vec4(texture(textureSampler, fragUv).rgb * fragColor * lighting, 1.0);
}