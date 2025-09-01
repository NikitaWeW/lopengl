#version 430

out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} fs_in;

layout(binding = 0) uniform sampler2D u_diffuse;

void main()
{
    o_color = texture(u_diffuse, fs_in.texCoords);
}