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
    o_color = vec4(0.08, 0.25, 0.13, 1.0) * max(0, dot(normalize(sunPos), normalize(fs_in.fragPos)));
}