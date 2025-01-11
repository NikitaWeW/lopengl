#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;

out vec3 v_color;
uniform vec2 u_offsets[100];

void main() {
    gl_Position = a_position + vec4(u_offsets[gl_InstanceID], 0, 0);
    v_color = a_color;
}

#shader fragment
#version 430 core
out vec4 o_color;

in vec3 v_color;

void main() {
    o_color = vec4(v_color, 1);
}