#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoords;
layout(location = 3) in vec4 a_tangent;

layout(std430) readonly buffer modelSSBO {
    mat4 modelMatrices[];
};

out VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
    mat3 TBN;
    vec3 viewPos;
    vec3 viewPosTangent;
    vec3 fragPositionTangent;
} vs_out;

// uniform mat4 u_modelMat;
// uniform mat4 u_normalMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

uniform vec3 u_viewPos;

void main() {
    mat4 normalMat = transpose(inverse(modelMatrices[gl_InstanceID]));
    gl_Position = u_projectionMat * u_viewMat * modelMatrices[gl_InstanceID] * a_position;
    vs_out.texCoords = a_texCoords;
    vs_out.fragPosition = vec3(modelMatrices[gl_InstanceID] * a_position);
    vs_out.normal = normalize(vec3(normalMat * a_normal));
    vs_out.viewPos = u_viewPos;

    vec3 tangent = normalize(vec3(normalMat * vec4(a_tangent.xyz, 0.0)));
    tangent = normalize(tangent - dot(tangent, vs_out.normal) * vs_out.normal);
    vec3 bitangent = cross(tangent, vs_out.normal);
    vs_out.TBN = mat3(tangent, bitangent, vs_out.normal);

    vs_out.fragPositionTangent = transpose(vs_out.TBN) * vs_out.fragPosition;
    vs_out.viewPosTangent = transpose(vs_out.TBN) * vs_out.viewPos;
}

#shader fragment
#version 430 core

#define LIGHTS_CAPASITY 50

struct Material {
    float shininess;

    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    
    bool specularSet;
    bool normalSet;
};
struct PointLight {
    vec3 position;

    vec3 color;

    float attenuation;
};

// https://www.youtube.com/watch?v=s2vGGAryxpM
in VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
    mat3 TBN;
    vec3 viewPos;
    vec3 viewPosTangent;
    vec3 fragPositionTangent;
} fs_in;

uniform Material u_material;

uniform PointLight       u_pointLights[LIGHTS_CAPASITY];

uniform int u_pointLightCount;

out vec4 o_color;

vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords);

void main() {
    vec3 viewDir = normalize(fs_in.viewPos - fs_in.fragPosition);

    vec2 texCoords = fs_in.texCoords;

    vec3 normal;
    if(u_material.normalSet) {
        normal = normalize(fs_in.TBN * normalize(texture(u_material.normal, texCoords).rgb * 2.0 - 1.0));
    } else {
        normal = fs_in.normal;
    }

    vec4 lightColor = vec4(0);

    for(int i = 0; i < u_pointLightCount; ++i) {
        lightColor += calculateLight(u_pointLights[i], u_material, normal, viewDir, texCoords);
    }

    o_color = (
        lightColor
    ) * texture(u_material.diffuse, texCoords);

    // gamma correction moved to post process for now
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
    o_color.a = 1;
}


vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords) {
    vec3 lightDir = normalize(light.position - fs_in.fragPosition);
    float distanceLightFragment = length(light.position - fs_in.fragPosition);
    float attenuation = 1.0 / (light.attenuation * distanceLightFragment * distanceLightFragment);

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
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), 125) * 
        (material.specularSet ? vec3(texture(material.specular, texCoords)) : vec3(.75));
    float shadow = 0;

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}