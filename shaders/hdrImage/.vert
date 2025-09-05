#version 330 core

vec2 vertices[3] = vec2[](
    vec2(-1,-1), 
    vec2(3,-1), 
    vec2(-1, 3)
);

out vec2 v_texCoord;

void main() {
    vec2 position = vertices[gl_VertexID];
    v_texCoord = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0, 1);
}
