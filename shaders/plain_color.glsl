#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
}

#shader fragment
#version 430 core
uniform vec3 u_color;

layout(location = 0) out vec4 o_position;
layout(location = 1) out vec4 o_normal;
layout(location = 2) out vec4 o_albedoSpecular;

void main() {
    o_albedoSpecular = vec4(u_color, 1);
}