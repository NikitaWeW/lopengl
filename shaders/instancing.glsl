#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in mat4 a_modelMat;

out vec2 v_texCoord;

uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * a_modelMat * a_position;
    v_texCoord = a_texCoord;
}

#shader fragment
#version 430 core
out vec4 o_color;

in vec2 v_texCoord;

struct Material {
    sampler2D diffuse;
};
uniform Material u_material;

void main() {
    o_color = texture(u_material.diffuse, v_texCoord);
}