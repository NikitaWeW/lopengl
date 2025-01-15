#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_modelMat;
uniform mat4 u_normalMat;
uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;
    
out VS_OUT {
    mat4 v_projectionMat;
    mat4 v_viewMat;
    vec2 v_texCoord;
    vec3 v_normal;
};

void main() {
    gl_Position = u_modelMat * a_position;
    v_texCoord = a_texCoord;
    v_projectionMat = u_projectionMat;
    v_viewMat = u_viewMat;
    v_normal = vec3(normalize(u_normalMat * a_normal));
}

#shader geometry
#version 430 core

in VS_OUT {
    mat4 v_projectionMat;
    mat4 v_viewMat;
    vec2 v_texCoord;
    vec3 v_normal;
} gs_in[];
out GS_OUT {
    vec2 v_texCoord;
};

uniform float u_timepoint;

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;
vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 5.0;
    vec3 direction = normal * ((sin(u_timepoint / 2) + 0.5)) * magnitude; 
    return position + vec4(direction, 0.0);
}

void main() {
    vec3 normal = gs_in[0].v_normal;

    gl_Position = gs_in[0].v_projectionMat * gs_in[0].v_viewMat * explode(gl_in[0].gl_Position, normal);
    v_texCoord = gs_in[0].v_texCoord;
    EmitVertex();
    gl_Position = gs_in[1].v_projectionMat * gs_in[1].v_viewMat * explode(gl_in[1].gl_Position, normal);
    v_texCoord = gs_in[1].v_texCoord;
    EmitVertex();
    gl_Position = gs_in[2].v_projectionMat * gs_in[2].v_viewMat * explode(gl_in[2].gl_Position, normal);
    v_texCoord = gs_in[2].v_texCoord;
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
    o_color = texture(u_material.diffuse, v_texCoord);
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2));
}