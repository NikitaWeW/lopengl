#version 430

out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} fs_in;

uniform vec3 sunPos;

void main()
{
    o_color = vec4(0.15, 0.62, 0.29, 1.0);
    o_color.xyz *= max(0, dot(normalize(sunPos), normalize(fs_in.fragPos)));
}