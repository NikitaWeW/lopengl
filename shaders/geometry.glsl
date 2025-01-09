#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_normalMat;
uniform mat4 u_projectionMat;

out VS_OUT {
    vec2 v_texCoord;
    vec3 v_normal;
};

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_texCoord = a_texCoord;
    v_normal = vec3(normalize(a_normal));
}

#shader geometry
#version 430 core

in VS_OUT {
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
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(u_timepoint) + 1.0) / 2) * magnitude; 
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}
void main() {
    vec3 normal = GetNormal(); // FIXME: gs_in[0].v_normal is wrong

    gl_Position = explode(gl_in[0].gl_Position, normal);
    v_texCoord = gs_in[0].v_texCoord;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    v_texCoord = gs_in[1].v_texCoord;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
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
    // o_color = vec4(1);
}