#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_view * u_model * a_position;
    v_texCoord = a_texCoord;
}

#shader fragment
#version 430 core
out vec4 color;

in vec2 v_texCoord;

uniform sampler2D texture_diffuse0;

void main() {
    vec4 texColor = texture(texture_diffuse0, v_texCoord);
    color = texColor;
}