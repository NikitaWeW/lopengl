#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoords;
layout(location = 5) in mat4 a_modelMat;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
} vs_out;

// uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * a_modelMat * a_position;
    vs_out.fragPosition = vec3(a_modelMat * a_position);
    vs_out.texCoords = a_texCoords;
    vs_out.normal = normalize(vec3(u_normalMat * a_normal));
}

#shader fragment
#version 430 core
out vec3 o_position;
out vec3 o_normal;
out vec4 o_albedoSpecular;

struct Material {
    float shininess;

    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    sampler2D height;
    
    bool specularSet;
    bool normalSet;
    bool heightSet;
};

in vec2 v_texCoord;
uniform Material u_material;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
} fs_in;

void main() {

    o_albedoSpecular.rgb = texture(u_material.diffuse, v_texCoord).rgb;
    o_albedoSpecular.a = u_material.specularSet ? texture(u_material.specular, v_texCoord) : 1;
    // moved to post process
    // o_color.rgb = pow(o_color.rgb, vec3(1/2.2));
}