#shader vertex
#version 430 core

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;

uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;

out VS_OUT {
    vec3 color;
};

void main() {
    gl_Position = u_projectionMat * u_viewMat * a_position;
    color = a_color;
}

#shader geometry
#version 430 core

layout(points) in;
// layout(lines, max_vertices = 2) out;
layout (triangle_strip, max_vertices = 5) out;

void main() {
    vec4 position = gl_in[0].gl_Position;
    float size = 0.5;

    // gl_Position = pos + vec4(size, 0, 0, 1);
    // EmitVertex();
    // gl_Position = pos + vec4(-size, 0, 0, 1);
    // EmitVertex();
    // EndPrimitive();


    
    
    gl_Position = position + vec4(-size, -size, 0, 1);
    EmitVertex();
    gl_Position = position + vec4(size, -size, 0, 1);
    EmitVertex();
    gl_Position = position + vec4(-size, size, 0, 1);
    EmitVertex();
    gl_Position = position + vec4(size, size, 0, 1);
    EmitVertex();
    gl_Position = position + vec4(0, size * 2, 0, 1);
    EmitVertex();
    EndPrimitive();
}

#shader fragment
#version 430 core

out vec4 o_color;

void main() {
    o_color = vec4(1, 1, 0, 1);
}