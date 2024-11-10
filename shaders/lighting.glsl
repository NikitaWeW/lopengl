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
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
struct Light {
    vec3 position;
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 color;

in vec3 v_normal;
in vec2 v_texCoord;
in vec3 fragPosition;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform vec3 u_viewPos;
uniform Material u_material;
uniform Light u_light;

void main() {
    vec4 textureColor = texture(texture_diffuse0, v_texCoord);

    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(u_light.position - fragPosition);
    vec3 viewDir = normalize(u_viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);

    vec3 ambient = u_light.ambient * u_light.color * u_material.ambient;
    vec3 diffuse = u_light.diffuse * u_light.color * vec3(max(dot(norm, lightDir), 0.0)) * u_material.diffuse;
    vec3 specular = u_light.specular * specularComponent * u_material.specular * u_light.color * vec3(texture(texture_specular0, v_texCoord));

    color = vec4(ambient + diffuse + specular, 1.0) * textureColor;
    // color = vec4(vec3(u_material.specular), 1.0);
}