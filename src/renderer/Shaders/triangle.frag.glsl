#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragLightPos;
layout(location = 4) in vec3 fragWorldPos;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 lightVP;
    vec4 camPos;
    vec4 sunDir;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D textureSampler;
layout(set = 1, binding = 0) uniform sampler2DShadow shadowMap;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// --- PBR Functions ---

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ACES Tone Mapping (more cinematic)
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
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(ubo.camPos.xyz - fragWorldPos);

    // Hardcoded material properties for now (could be passed via push constants or another UBO)
    vec3 albedo = texture(textureSampler, fragUv).rgb * fragColor;
    float metallic = 0.5;
    float roughness = 0.5;
    float ao = 1.0;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Light source (Directional)
    vec3 L = normalize(ubo.sunDir.xyz);
    vec3 H = normalize(V + L);
    vec3 radiance = vec3(1.5); // Slightly stronger for HDR feel

    // Shadow calculation
    vec3 projCoords = fragLightPos.xyz / fragLightPos.w;
    projCoords = projCoords * 0.5 + 0.5;
    float shadow = 1.0;
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
        projCoords.z <= 1.0)
    {
        shadow = texture(shadowMap, vec3(projCoords.xy, projCoords.z));
    }

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + (kD * albedo / PI + specular) * radiance * NdotL * shadow;

    outColor = vec4(color, 1.0);
}