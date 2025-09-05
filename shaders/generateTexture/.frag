#version 430

in vec2 v_texCoord;
out vec3 o_color;

uniform vec2 u_dimensions;
uniform uint u_face;

void main() 
{
    o_color.rgb = vec3(1);
}