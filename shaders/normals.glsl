#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;

uniform mat4 u_modelMat;
uniform mat4 u_normalMat;
uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;
    
out VS_OUT {
    mat4 v_projectionMat;
    mat4 v_viewMat;
    vec3 v_normal;
};

void main() {
    gl_Position = u_modelMat * a_position;
    v_projectionMat = u_projectionMat;
    v_viewMat = u_viewMat;
    v_normal = vec3(normalize(u_normalMat * a_normal));
}

#shader geometry
#version 430 core

in VS_OUT {
    mat4 v_projectionMat;
    mat4 v_viewMat;
    vec3 v_normal;
} gs_in[];

uniform float u_timepoint;

layout (triangles) in;
layout (line_strip, max_vertices=6) out;

void main() {
    for(int i = 0; i < 3; ++i) {
        gl_Position = gs_in[i].v_projectionMat * gs_in[i].v_viewMat * gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = gs_in[i].v_projectionMat * gs_in[i].v_viewMat * (gl_in[i].gl_Position + vec4(gs_in[i].v_normal, 0));
        EmitVertex();
        EndPrimitive();
    }
}

#shader fragment
#version 430 core
out vec4 o_color;

void main() {
    o_color = vec4(1, 1, 0, 1);
}