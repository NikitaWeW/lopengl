#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out VS_OUT {
    vec2 v_texCoord;
    vec4 v_fragPosition;
    vec3 v_normal;
    mat4 v_viewMat;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform mat4 u_normalMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    vs_out.v_texCoord = a_texCoord;
    vs_out.v_fragPosition = u_modelMat * a_position;
    vs_out.v_normal = vec3(u_normalMat * a_normal);
    vs_out.v_viewMat = u_viewMat;
}

#shader fragment
// its horrible
#version 430 core

out vec4 o_color;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct PointLight {
    vec3 position;

    vec3 color;

    float constant;
    float linear;
    float quadratic;

    mat4 projectionMat;
    mat4 viewMat;
};
struct SpotLight {
    vec3 position;
    vec3 direction;

    float innerCutoff;
    float outerCutoff;

    vec3 color;

    float constant;
    float linear;
    float quadratic;

    mat4 projectionMat;
    mat4 viewMat;
};
struct DirectionalLight {
    vec3 direction;
    vec3 color;

    mat4 projectionMat;
    mat4 viewMat;
};

in VS_OUT {
    vec2 v_texCoord;
    vec4 v_fragPosition;
    vec3 v_normal;
    mat4 v_viewMat;
} fs_in;

#define LIGHTS_CAPASITY 10

uniform Material u_material;

uniform SpotLight        u_spotLights [LIGHTS_CAPASITY];
uniform DirectionalLight u_dirLights  [LIGHTS_CAPASITY];
uniform PointLight       u_pointLights[LIGHTS_CAPASITY];

uniform int u_pointLightCount;
uniform int u_dirLightCount;
uniform int u_spotLightCount;
uniform vec3 u_viewPos;
uniform bool u_specularSet;

uniform sampler2D u_depthMap; // TODO: move in the light struct, manage by renderer

vec4 light(PointLight light, Material material, vec3 norm, vec3 viewDir);
vec4 light(DirectionalLight light, Material material, vec3 norm, vec3 viewDir);
vec4 light(SpotLight light, Material material, vec3 norm, vec3 viewDir);
float shadow(vec4 fragPosLightSpace, sampler2D depthMap);

void main() { // TODO: displacement
    vec4 lightColor = vec4(0, 0, 0, 1);

    vec3 norm = normalize(fs_in.v_normal);
    vec3 viewDir = normalize(u_viewPos - vec3(fs_in.v_fragPosition));

    for(int i = 0; i < u_pointLightCount; ++i) {
        lightColor += light(u_pointLights[i], u_material, norm, viewDir);
    }
    for(int i = 0; i < u_dirLightCount; ++i) {
        lightColor += light(u_dirLights[i], u_material, norm, viewDir);
    }
    for(int i = 0; i < u_spotLightCount; ++i) {
        lightColor += light(u_spotLights[i], u_material, norm, viewDir);
    }
    o_color = lightColor * texture(u_material.diffuse, fs_in.v_texCoord);
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}

vec4 light(PointLight light, Material material, vec3 norm, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - vec3(fs_in.v_fragPosition));
    vec3 reflectDir = reflect(-lightDir, norm);
    float distanceLightFragment = length(light.position - vec3(fs_in.v_fragPosition));
    float attenuation = 1.0 / (light.constant + light.linear * distanceLightFragment + light.quadratic * distanceLightFragment * distanceLightFragment);

    vec3 ambient = 
        light.color * 0.125 * 
        attenuation;
    vec3 diffuse = 
        light.color * 
        attenuation *
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.color * 
        attenuation *
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), u_material.shininess) * 
        (u_specularSet ? vec3(texture(material.specular, fs_in.v_texCoord)) : vec3(.25));
    float shadow = shadow(light.projectionMat * light.viewMat * fs_in.v_fragPosition, u_depthMap);

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}
vec4 light(DirectionalLight light, Material material, vec3 norm, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 ambient = light.color * 0.125;
    vec3 diffuse = 
        light.color * 
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.color * 
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), u_material.shininess) * 
        (u_specularSet ? vec3(texture(material.specular, fs_in.v_texCoord)) : vec3(.25));
    float shadow = shadow(light.projectionMat * light.viewMat * fs_in.v_fragPosition, u_depthMap);

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}
vec4 light(SpotLight light, Material material, vec3 norm, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - vec3(fs_in.v_fragPosition));
    vec3 reflectDir = reflect(-lightDir, norm);
    float distanceLightFragment = length(light.position - vec3(fs_in.v_fragPosition));
    float attenuation = 1.0 / (light.constant + light.linear * distanceLightFragment + light.quadratic * distanceLightFragment * distanceLightFragment);

    vec3 ambient = 
        light.color * 0.125 * 
        attenuation;
    float theta = dot(lightDir, normalize(-light.direction));
    if(theta > light.outerCutoff) {
        float epsilon = light.innerCutoff - light.outerCutoff;
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

        vec3 diffuse = 
            light.color * 
            intensity *
            attenuation *
            vec3(max(dot(norm, lightDir), 0.0));
        vec3 specular = 
            light.color *
            intensity * 
            attenuation *
            pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), u_material.shininess) * 
            (u_specularSet ? vec3(texture(material.specular, fs_in.v_texCoord)) : vec3(.25));
        float shadow = shadow(light.projectionMat * light.viewMat * fs_in.v_fragPosition, u_depthMap);

        return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
    } else {
        return vec4(ambient, 1.0); // unsure about that
    }
}

float shadow(vec4 fragPosLightSpace, sampler2D depthMap) {
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projectedCoords = projectedCoords * 0.5 + 0.5;

    float closestDepth = texture(depthMap, projectedCoords.xy).r;
    float currentDepth = projectedCoords.z;

    return currentDepth > closestDepth ? 1.0 : 0.0;
}