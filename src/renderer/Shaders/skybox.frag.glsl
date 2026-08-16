#version 450

layout(location = 0) in vec3 localPos;

layout(push_constant) uniform SkyboxPush {
    mat4 invViewProj;
    vec4 sunDir;
} push;

layout(set = 0, binding = 0) uniform samplerCube skybox;

layout(location = 0) out vec4 outColor;

vec3 getSkyColor(vec3 dir, vec3 sunDir)
{
    const vec3 zenith = vec3(0.05, 0.12, 0.35);
    const vec3 horizon = vec3(0.62, 0.76, 0.92);

    float f = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 color = mix(horizon, zenith, pow(f, 1.5));

    // Sun disc
    float sunFocus = max(dot(normalize(dir), sunDir), 0.0);
    if (sunFocus > 0.996)
    {
        color = mix(color, vec3(1.0, 0.96, 0.85), smoothstep(0.996, 0.999, sunFocus));
    }

    // Simple glow around sun
    color += vec3(1.0, 0.8, 0.5) * pow(sunFocus, 256.0);

    return color;
}

void main()
{
    vec3 dir = normalize(localPos);
    vec3 color = getSkyColor(dir, normalize(push.sunDir.xyz));

    // Sample static cubemap for additional details or fallback
    vec3 cubeColor = texture(skybox, dir).rgb;
    color = mix(color, cubeColor, 0.1); // Blend 10% of the static map if needed

    outColor = vec4(color, 1.0);
}
