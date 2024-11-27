#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;
out vec3 fragPosition;
out vec3 v_normal;
out mat4 v_viewMat;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform mat4 u_normalMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_texCoord = a_texCoord;
    fragPosition = vec3(a_position * u_modelMat);
    v_normal = vec3(u_normalMat * a_normal);
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
struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
struct SpotLight {
    vec3 position;
    vec3 direction;

    float innerCutoff;
    float outerCutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

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

in vec3 v_normal;
in vec2 v_texCoord;
in vec3 fragPosition;
in mat4 v_viewMat;

#define LIGHTS_CAPASITY 10

uniform Material u_material;

uniform SpotLight        u_spotLights [LIGHTS_CAPASITY];
uniform DirectionalLight u_dirLights  [LIGHTS_CAPASITY];
uniform PointLight       u_pointLights[LIGHTS_CAPASITY];

uniform int u_pointLightCount;
uniform int u_dirLightCount;
uniform int u_spotLightCount;
uniform vec3 u_viewPos;

void main() {
    vec4 textureColor = texture(u_material.diffuse, v_texCoord);

    vec4 color = vec4(0, 0, 0, 1);
    if(u_spotLightCount == 0 && u_dirLightCount == 0 && u_pointLightCount == 0) {
        return;
    }

    vec4 specularColor = texture(u_material.specular, v_texCoord);
    vec3 norm = normalize(v_normal);a
    vec3 viewDir = normalize(u_viewPos - fragPosition);

    for(int i = 0; i < u_pointLightCount; ++i) {
        PointLight light = u_pointLights[i];
        vec3 lightDir = normalize(light.position - fragPosition);
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

        color = color + vec4(ambient + diffuse + specular, 1.0);
        // color += vec4(0.25, 0, 0, 1.0);
    }
    // for(int i = 0; i < u_dirLightCount; ++i) {
    //     DirectionalLight light = u_dirLights[i];
    //     vec3 lightDir = normalize(-light.direction);
    //     vec3 reflectDir = reflect(-lightDir, norm);

    //     vec3 ambient = light.ambient;
    //     vec3 diffuse = 
    //         light.diffuse * 
    //         vec3(max(dot(norm, lightDir), 0.0));
    //     vec3 specular = 
    //         light.specular * 
    //         pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess) * 
    //         (vec3(specularColor) == vec3(0) ? vec3(.25) : vec3(specularColor));

    //     color += vec4(ambient + diffuse + specular, 1.0);
    // }
    // for(int i = 0; i < u_spotLightCount; ++i) {
    //     SpotLight light = u_spotLights[i];
    //     vec3 lightDir = normalize(light.position - fragPosition);
    //     vec3 reflectDir = reflect(-lightDir, norm);
    //     float distanceLightFragment = length(light.position - fragPosition);
    //     float attenuation = 1.0 / (light.constant + light.linear * distanceLightFragment + light.quadratic * distanceLightFragment * distanceLightFragment);

    //     vec3 ambient = 
    //         light.ambient * 
    //         attenuation;
    //     float theta = dot(lightDir, normalize(-light.direction));
    //     if(theta > light.outerCutoff) {
    //         float epsilon = light.innerCutoff - light.outerCutoff;
    //         float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    //         vec3 diffuse = 
    //             light.diffuse * 
    //             intensity *
    //             attenuation *
    //             vec3(max(dot(norm, lightDir), 0.0));
    //         vec3 specular = 
    //             light.specular *
    //             intensity * 
    //             attenuation *
    //             pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess) * 
    //             (vec3(specularColor) == vec3(0) ? vec3(.25) : vec3(specularColor));
    //         color += vec4(ambient + diffuse + specular, 1.0);
    //     } else {
    //         color += vec4(ambient, 1.0);
    //     }
    // }
// color = vec4(vec3(fragPosition), 1.0);
// color = textureColor;
// color = vec4(vec3(clamp(u_pointLightCount, 0, 2) * 0.5), 1.0);

    o_color = color;
}