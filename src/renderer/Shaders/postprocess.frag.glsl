#version 450

layout(location = 0) in vec2 inUv;

layout(set = 0, binding = 0, input_attachment_index = 0) uniform subpassInput inputTexture;

layout(location = 0) out vec4 outColor;

// ACES Tone Mapping
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    // Simulated Foveated Rendering: pixelate edges to show resolution drop
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(inUv, center);

    vec3 color = subpassLoad(inputTexture).rgb;

    // To see the effect, we just tint the edges since subpassLoad doesn't support custom UVs
    if (dist > 0.3) {
        color = mix(color, vec3(1.0, 0.0, 0.0), (dist - 0.3) * 0.3); // Red tint on edges
    }

    // Tone Mapping
    color = ACESFilm(color);

    // Gamma Correction
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}
