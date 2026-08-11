#version 450

layout(push_constant) uniform InvViewProj {
    mat4 invViewProj;
} push;

layout(location = 0) out vec3 localPos;

void main()
{
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 clip = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 p = push.invViewProj * clip;
    localPos = p.xyz / p.w;
    gl_Position = clip;
}
