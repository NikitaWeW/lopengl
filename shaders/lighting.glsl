#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;
out vec3 fragPosition;
out vec3 v_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_normalMat;

void main() {
    gl_Position = u_projection * u_view * u_model * a_position;
    v_texCoord = a_texCoord;
    fragPosition = vec3(u_model * a_position);
    v_normal = vec3(u_normalMat * a_normal);
}

#shader fragment
#version 430 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 color;

in vec3 v_normal;
in vec2 v_texCoord;
in vec3 fragPosition;

uniform vec3 u_viewPos;
uniform Material u_material;
uniform Light u_light;

void main() {
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(u_light.position - fragPosition);
    vec3 viewDir = normalize(u_viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 ambient = u_light.ambient;
    vec3 diffuse = 
        u_light.diffuse * 
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        u_light.specular * 
        pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess) * 
        vec3(texture(u_material.specular, v_texCoord));

    color = vec4(ambient + diffuse + specular, 1.0) * texture(u_material.diffuse, v_texCoord);
    // color = vec4(vec3(somevalue), 1.0f);
    // color = vec4(vec3(u_material.shininess == 32 ? 1. : 0.), 1.0f);
    // color = vec4(vec3(texture(u_material.diffuse, v_texCoord)), 1.0f);
}