#shader vertex
#version 430 core
layout (location = 0) in vec4 a_position;
layout (location = 2) in vec2 a_texCoords;

out vec2 v_texCoords;

void main() {
    gl_Position = a_position;
    v_texCoords = a_texCoords;
}

#shader fragment
#version 430 core
in vec2 v_texCoords;

uniform sampler2D u_texture;
uniform bool u_horizontal;

out vec4 o_color;

void main() {
    vec2 texelSize = 1 / textureSize(u_texture, 0);
    vec3 result = texture()

    o_color = vec4(1,1,1, 1);
}