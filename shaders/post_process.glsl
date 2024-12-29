#shader vertex
#version 430 core
layout (location = 0) in vec3 a_position;
// normals at location = 1
layout (location = 2) in vec2 a_texCoords;

out vec2 v_texCoords;

void main() {
    gl_Position = vec4(a_position.xy, 0, 1);
    v_texCoords = a_texCoords;
}

#shader fragment
#version 430 core
in vec2 v_texCoords;
uniform sampler2D u_texture;

out vec4 o_color;

void main() {
    o_color = texture(u_texture, v_texCoords);
}