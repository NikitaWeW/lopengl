#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_view * u_model * a_position;
}

#shader fragment
#version 430 core
out vec4 color;

void main() {
    color = vec4(1.0);
}