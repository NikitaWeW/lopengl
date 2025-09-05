#version 430 core
out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} fs_in;

layout (binding = 0) uniform samplerCube u_texture;

void main() 
{
    o_color.rgb = texture(u_texture, normalize(fs_in.fragPos)).rgb;
    o_color.a = 1;
}
