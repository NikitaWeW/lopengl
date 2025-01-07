#shader vertex
#version 430 core

layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * a_position;
    v_texCoord = a_texCoord;
}

#shader geometry
#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 5) out;

void buildHouse(vec4 pos, float size) {
    gl_Position = pos + vec4(-size, -size);
    EmitVertex();

    gl_Position = pos + vec4(-size, size);
    EmitVertex();
    
    gl_Position = pos + vec4(size, -size);
    EmitVertex();
    
    gl_Position = pos + vec4(size, size);
    EmitVertex();
    
    gl_Position = pos + vec4(size / 2, size * 2);
    EmitVertex();
}

void main() {
    buildHouse(gl_in[0].gl_Position);
}

#shader fragment
#version 430 core

in vec2 v_texCoord;
out vec4 o_color;

void main() {
    o_color = vec4(v_texCoord, 0, 1);
}