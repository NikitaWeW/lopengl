#version 430

out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} fs_in;

uniform vec3 sunPos;

layout(binding = 0) uniform samplerCube u_texture;

void main()
{
    o_color = texture(u_texture, fs_in.fragPos);
    o_color.xyz *= max(0.1, dot(normalize(sunPos), normalize(fs_in.fragPos)));
}