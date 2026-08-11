#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragLightPos;

layout(set = 0, binding = 1) uniform sampler2D textureSampler;
layout(set = 1, binding = 0) uniform sampler2DShadow shadowMap;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.4));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);

    vec3 projCoords = fragLightPos.xyz / fragLightPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    float shadow = 1.0;
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
        projCoords.z <= 1.0)
    {
        shadow = texture(shadowMap, vec3(projCoords.xy, projCoords.z));
    }

    outColor = vec4(texture(textureSampler, fragUv).rgb * fragColor *
                        (0.15 + 0.85 * diffuse * shadow),
                    1.0);
}