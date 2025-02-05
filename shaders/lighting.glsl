#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoord;

out VS_OUT {
    vec2 texCoords;
    vec4 fragPosition;
    vec3 normal;
    mat4 viewMat;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform mat4 u_normalMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    vs_out.texCoords = a_texCoord;
    vs_out.fragPosition = u_modelMat * a_position;
    vs_out.normal = normalize(vec3(u_normalMat * a_normal));
    vs_out.viewMat = u_viewMat;
}

#shader fragment
#version 430 core

#define LIGHTS_CAPASITY 10

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

    samplerCube depthMap;
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
    
    sampler2D depthMap;
};
struct DirectionalLight {
    vec3 direction;
    vec3 color;

    mat4 projectionMat;
    mat4 viewMat;
    
    sampler2D depthMap;
};

in VS_OUT {
    vec2 texCoords;
    vec4 fragPosition;
    vec3 normal;
    mat4 viewMat;
} fs_in;

uniform Material u_material;

uniform SpotLight        u_spotLights [LIGHTS_CAPASITY];
uniform DirectionalLight u_dirLights  [LIGHTS_CAPASITY];
uniform PointLight       u_pointLights[LIGHTS_CAPASITY];

uniform int u_pointLightCount;
uniform int u_dirLightCount;
uniform int u_spotLightCount;
uniform vec3 u_viewPos;
uniform bool u_specularSet;

out vec4 o_color;

vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir);
vec4 calculateLight(DirectionalLight light, Material material, vec3 norm, vec3 viewDir);
vec4 calculateLight(SpotLight light, Material material, vec3 norm, vec3 viewDir);
float calculateShadow(PointLight light);
float calculateShadow(DirectionalLight light);
float calculateShadow(SpotLight light);

void main() {
    vec3 viewDir = normalize(u_viewPos - vec3(fs_in.fragPosition));

    o_color = (
        calculateLight(u_pointLights[0], u_material, fs_in.normal, viewDir) + 
        calculateLight(u_dirLights[0], u_material, fs_in.normal, viewDir)
    ) * texture(u_material.diffuse, fs_in.texCoords);
// o_color = calculateLight(u_dirLights[0], u_material, fs_in.normal, viewDir);
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}


vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - vec3(fs_in.fragPosition));
    float distanceLightFragment = length(light.position - vec3(fs_in.fragPosition));
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
        (u_specularSet ? vec3(texture(material.specular, fs_in.texCoords)) : vec3(.25));
    float shadow = calculateShadow(light);

    return vec4(ambient + vec3(1 - shadow) * (diffuse + specular), 1.0);
}
vec4 calculateLight(DirectionalLight light, Material material, vec3 norm, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient = light.color * 0.125;
    vec3 diffuse = 
        light.color * 
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.color * 
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), u_material.shininess) * 
        (u_specularSet ? vec3(texture(material.specular, fs_in.texCoords)) : vec3(.25));
    float shadow = calculateShadow(light);
// return vec4(vec3(shadow), 1);
    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}
vec4 calculateLight(SpotLight light, Material material, vec3 norm, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - vec3(fs_in.fragPosition));
    float distanceLightFragment = length(light.position - vec3(fs_in.fragPosition));
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
            (u_specularSet ? vec3(texture(material.specular, fs_in.texCoords)) : vec3(.25));
        float shadow = calculateShadow(light);

        return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
    } else {
        return vec4(ambient, 1.0); // unsure about that
    }
}

float calculateShadow(PointLight light) {
    vec3 fragToLight = vec3(fs_in.fragPosition.xyz - light.position);
    float closestDepth = texture(light.depthMap, fragToLight).r * 100; // 100 -- far plane
    float currentDepth = length(fragToLight);

    float bias = 0.05;
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}
float calculateShadow(DirectionalLight light) {
    vec4 fragPosLightSpace = light.projectionMat * light.viewMat * fs_in.fragPosition;
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projectedCoords = projectedCoords * 0.5 + 0.5;
    if(projectedCoords.z > 0.99) return 0;

    float bias = min(-dot(normalize(light.direction) * 0.01, fs_in.normal), 0.0009);
    float currentDepth = projectedCoords.z;

    float closestDepth = texture(light.depthMap, projectedCoords.xy).r;
    // return currentDepth - bias > closestDepth ? 1.0 : 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.depthMap, 0);
    int numSamples = 0;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(light.depthMap, projectedCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
            ++numSamples;
        }
    }
    shadow /= numSamples;
    return shadow;
}
float calculateShadow(SpotLight light) {
    return 0; // nah
}