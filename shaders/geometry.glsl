#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;


out VS_OUT {
    vec2 v_texCoord;
};

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_texCoord = a_texCoord;
}

#shader geometry
#version 430 core

in VS_OUT {
    vec2 v_texCoord;
} gs_in[];
out GS_OUT {
    vec2 v_texCoord;
};

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;
void main() {
    v_texCoord = gs_in[0].v_texCoord;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    v_texCoord = gs_in[1].v_texCoord;
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    v_texCoord = gs_in[2].v_texCoord;
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}

#shader fragment
#version 430 core
out vec4 o_color;

struct Material {
    sampler2D diffuse;
};
in GS_OUT {
    vec2 v_texCoord;
};

uniform Material u_material;

void main() {
    // o_color = texture(u_material.diffuse, v_texCoord);
    o_color = vec4(1);
}