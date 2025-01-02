#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;
out vec3 v_fragPosition;
out vec3 v_normal;
out mat4 v_viewMat;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform mat4 u_normalMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_texCoord = a_texCoord;
    v_fragPosition = vec3(u_modelMat * a_position);
    v_normal = normalize(vec3(u_normalMat * a_normal));
    v_viewMat = u_viewMat;
}

#shader fragment
#version 430 core

out vec4 o_color;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_fragPosition;
in mat4 v_viewMat;

uniform Material u_material;
uniform samplerCube u_skybox;
uniform vec3 u_viewPos;
uniform bool u_specularSet;

void main() {
    float ratio = 1 / 1.33;
    vec3 viewToFrag = normalize(v_fragPosition - u_viewPos);
    vec3 refractionDir = refract(viewToFrag, v_normal, ratio);
    o_color = texture(u_skybox, refractionDir);
}