#version 430 core
out vec4 o_color;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    vec3 normal;
} fs_in;

void main() 
{
    o_color = vec4(1);
}
