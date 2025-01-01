#shader vertex
#version 430 core

layout (location = 0) in vec3 a_position;

out vec3 v_texCoords;

uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;

void main() {
    v_texCoords = a_position;
    gl_Position = (u_projectionMat * mat4(mat3(u_viewMat)) * vec4(a_position, 1)).xyzw;
}

#shader fragment
#version 430 core


in vec3 v_texCoords;

out vec4 o_color;

uniform samplerCube skybox;

void main() {
    o_color = texture(skybox, v_texCoords);
}