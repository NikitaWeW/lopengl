#version 430 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    mat3 TBN;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    vs_out.fragPos = vec3(u_modelMat * vec4(a_position, 1));
    gl_Position = u_projectionMat * u_viewMat * vec4(vs_out.fragPos, 1);
    vs_out.texCoords = a_texCoord;
    
    mat4 normalMat = transpose(inverse(u_modelMat));
    vec3 normal = normalize(vs_out.fragPos);
    // normal = normalize(vec3(normalMat * vec4(normal, 0)));
    
    vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = normalize(cross(normal, tangent));

    vs_out.TBN = mat3(tangent, bitangent, normal);
}
