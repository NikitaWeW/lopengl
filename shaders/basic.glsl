#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_texCoord = a_texCoord;
}

#shader fragment
#version 430 core
out vec4 o_color;

struct Material {
    sampler2D diffuse;
};

in vec2 v_texCoord;
uniform Material u_material;

void main() {
    o_color = texture(u_material.diffuse, v_texCoord);
}