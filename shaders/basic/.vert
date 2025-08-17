#version 330 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec4 a_tangent;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_normalMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    vs_out.fragPos = vec3(u_modelMat * a_position);
    gl_Position = u_projectionMat * u_viewMat * vec4(vs_out.fragPos, 1);
    vs_out.texCoords = a_texCoord;
    
    vec3 normal = normalize(vec3(u_normalMat * a_normal));
    vec3 tangent = normalize(vec3(u_normalMat * vec4(a_tangent.xyz, 0.0)));
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);
    vs_out.TBN = mat3(tangent, bitangent, normal);
}
