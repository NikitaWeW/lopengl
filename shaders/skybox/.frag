#version 430 core

in vec3 v_texCoords;
layout (binding = 0) uniform samplerCube u_skybox;
out vec4 o_color;

void main() 
{
    o_color = texture(u_skybox, v_texCoords);
}