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
struct PointLight {
    vec3 position;

// colors
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

// attenuation
    float constant;
    float linear;
    float quadratic;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    float innerCutoff;
    float outerCutoff;

// colors
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

// attenuation
    float constant;
    float linear;
    float quadratic;
};
struct DirectionalLight {
    vec3 direction;
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
uniform SpotLight u_light;

vec3 processLight(PointLight light);
vec3 processLight(DirectionalLight light);
vec3 processLight(SpotLight light);

void main() {
    vec4 textureColor = texture(u_material.diffuse, v_texCoord);

    color = vec4(processLight(u_light), 1.0) * textureColor;
    // color = vec4(normalize(v_normal), 1.);
    // color = textureColor;
    // color = vec4(processLight(u_light), 1.0);
    // color = vec4(vec3(vec3(texture(u_material.specular, v_texCoord))), 1.0f);
}

vec3 processLight(PointLight light) 
{
    vec4 specularColor = texture(u_material.specular, v_texCoord);
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(light.position - fragPosition);
    vec3 viewDir = normalize(u_viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float distanceLightFragment = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distanceLightFragment + light.quadratic * distanceLightFragment * distanceLightFragment);

    vec3 ambient = 
        light.ambient * 
        attenuation;
    vec3 diffuse = 
        light.diffuse * 
        attenuation *
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.specular * 
        attenuation *
        pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess) * 
        (vec3(specularColor) == vec3(0) ? vec3(.25) : vec3(specularColor));
    return ambient + diffuse + specular;
}
vec3 processLight(DirectionalLight light)
{
    vec4 specularColor = texture(u_material.specular, v_texCoord);
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(-light.direction);
    vec3 viewDir = normalize(u_viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 ambient = light.ambient;
    vec3 diffuse = 
        light.diffuse * 
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.specular * 
        pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess) * 
        (vec3(specularColor) == vec3(0) ? vec3(.25) : vec3(specularColor));
    return ambient + diffuse + specular;
}

vec3 processLight(SpotLight light)
{
    vec4 specularColor = texture(u_material.specular, v_texCoord);
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(light.position - fragPosition);
    vec3 viewDir = normalize(u_viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float distanceLightFragment = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distanceLightFragment + light.quadratic * distanceLightFragment * distanceLightFragment);

    vec3 ambient = 
        light.ambient * 
        attenuation;
    float theta = dot(lightDir, normalize(-light.direction));
    if(theta > light.outerCutoff) {
        float epsilon = light.innerCutoff - light.outerCutoff;
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

        vec3 diffuse = 
            light.diffuse * 
            intensity *
            attenuation *
            vec3(max(dot(norm, lightDir), 0.0));
        vec3 specular = 
            light.specular *
            intensity * 
            attenuation *
            pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess) * 
            (vec3(specularColor) == vec3(0) ? vec3(.25) : vec3(specularColor));
        return ambient + diffuse + specular;
    } else {
        return ambient;
    }
}