#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoords;
layout(location = 3) in vec4 a_tangent;
// layout(location = 5) in mat4 a_modelMat;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
    mat3 TBN;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_normalMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    // mat4 normalMat = transpose(inverse(a_modelMat));
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    vs_out.fragPosition = vec3(u_modelMat * a_position);
    vs_out.texCoords = a_texCoords;
    vs_out.normal = normalize(vec3(u_normalMat * a_normal));

    vec3 tangent = normalize(vec3(u_normalMat * vec4(a_tangent.xyz, 0.0)));
    tangent = normalize(tangent - dot(tangent, vs_out.normal) * vs_out.normal);
    vec3 bitangent = cross(tangent, vs_out.normal);
    vs_out.TBN = mat3(tangent, bitangent, vs_out.normal);
}

#shader fragment
#version 430 core
layout(location = 0) out vec4 o_position;
layout(location = 1) out vec4 o_normal;
layout(location = 2) out vec4 o_albedoSpecular;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    
    bool specularSet;
    bool normalSet;
};

uniform Material u_material;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
    mat3 TBN;
} fs_in;

void main() {
    o_position = vec4(fs_in.fragPosition, 1);
    if(u_material.normalSet) {
        o_normal = vec4(normalize(fs_in.TBN * normalize(texture(u_material.normal, fs_in.texCoords).rgb)), 1);
    } else {
        o_normal = vec4(fs_in.normal, 1);
    }
    o_albedoSpecular.rgb = texture(u_material.diffuse, fs_in.texCoords).rgb;
    o_albedoSpecular.a = u_material.specularSet ? texture(u_material.specular, fs_in.texCoords).r : 1;
}