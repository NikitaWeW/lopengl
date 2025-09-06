#version 330 core

vec2 vertices[4] = vec2[](
    vec2(-1, 1), 
    vec2( 1, 1),
    vec2(-1,-1), 
    vec2( 1,-1)
);

out vec2 v_texCoord;

void main() {
    vec2 position = vertices[gl_VertexID];
    v_texCoord = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0, 1);
}
