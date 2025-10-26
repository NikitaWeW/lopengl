#version 430

out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    mat3 TBN;
} fs_in;

layout(binding = 0) uniform samplerCube u_diffuse;
layout(binding = 1) uniform samplerCube u_normal;

void main()
{
    vec3 normal = normalize(fs_in.TBN * texture(u_normal, fs_in.fragPos).rgb * 2.0 - 1.0);
    vec3 color = texture(u_diffuse, fs_in.fragPos).rgb;

    const vec3 sunDir = normalize(vec3(1, -1, -1));
    o_color.rgb = max(0.1, dot(-sunDir, normal)) * color;
    o_color.rgb = normal;
    o_color.a = 1;
}