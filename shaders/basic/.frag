#version 430

out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} fs_in;

void main()
{
    o_color = vec4(0.17, 0.71, 0.3, 1.0);
}