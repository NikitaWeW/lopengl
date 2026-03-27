#version 430 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} vs_out;

uniform mat4 uModelMat;
uniform mat4 uViewMat;
uniform mat4 uProjMat;

void main() {
    vs_out.fragPos = vec3(uModelMat * vec4(aPosition, 1));
    gl_Position = uProjMat * uViewMat * vec4(vs_out.fragPos, 1);
    vs_out.texCoords = aTexCoord;
    
    mat4 normalMat = transpose(inverse(uModelMat));
    vec3 normal = normalize(vec3(normalMat * vec4(aNormal, 0)));
    vec3 tangent = normalize(vec3(normalMat * vec4(aTangent.xyz, 0.0)));
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);
    vs_out.TBN = mat3(tangent, bitangent, normal);
}
