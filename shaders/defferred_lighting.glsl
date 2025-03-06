#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoords;

out VS_OUT {
    vec2 texCoords;
} vs_out;

void main() {
    gl_Position = a_position;
    vs_out.texCoords = a_texCoords;
}

#shader fragment
#version 430 core

#define LIGHTS_CAPASITY 50

struct Material {
    sampler2D position;
    sampler2D normal;
    sampler2D albedoSpecular;
};
struct PointLight {
    vec3 position;

    vec3 color;

    float attenuation;
};

in VS_OUT {
    vec2 texCoords;
} fs_in;

uniform Material u_material;

uniform PointLight u_pointLights[LIGHTS_CAPASITY];
uniform int u_pointLightCount;

uniform vec3 u_viewPos;

out vec4 o_color;

vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords, vec3 fragPosition);

void main() {
    vec2 texCoords = fs_in.texCoords;

    vec3 fragPosition = texture(u_material.position, texCoords).rgb;
    vec3 viewDir = normalize(u_viewPos - fragPosition);

    vec3 normal = texture(u_material.normal, texCoords).rgb;

    vec4 lightColor = vec4(0);
    for(int i = 0; i < u_pointLightCount; ++i) {
        lightColor += calculateLight(u_pointLights[i], u_material, normal, viewDir, texCoords, fragPosition);
    }

    o_color = (
        lightColor
    ) * vec4(texture(u_material.albedoSpecular, texCoords).rgb, 1);

    // gamma correction moved to post process for now
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
    o_color.a = 1;
}


vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords, vec3 fragPosition) {
    vec3 lightDir = normalize(light.position - fragPosition);
    float distanceLightFragment = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.attenuation * distanceLightFragment * distanceLightFragment);

    vec3 ambient = 
        light.color * 0.01 *
        attenuation;
    vec3 diffuse = 
        light.color * 
        attenuation *
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.color * 
        attenuation *
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), 64) * 
        texture(material.albedoSpecular, texCoords).a;
    float shadow = 0;

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}