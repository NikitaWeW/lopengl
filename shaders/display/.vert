#version 330 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    vec3 normal;
} vs_out;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * a_position;
    vs_out.texCoords = a_texCoord;
    vs_out.fragPos = vec3(a_position);
    
    vs_out.normal = normalize(vec3(a_normal));
}
