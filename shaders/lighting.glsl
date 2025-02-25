#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_texCoords;
layout(location = 3) in vec4 a_tangent;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPosition;
    vec3 normal;
    mat3 TBN;
    vec3 viewPos;
    vec3 viewPosTangent;
    vec3 fragPositionTangent;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform mat4 u_normalMat;

uniform vec3 u_viewPos;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    vs_out.texCoords = a_texCoords;
    vs_out.fragPosition = vec3(u_modelMat * a_position);
    vs_out.normal = normalize(vec3(u_normalMat * a_normal));
    vs_out.viewPos = u_viewPos;

    vec3 tangent = normalize(vec3(u_normalMat * vec4(a_tangent.xyz, 0.0)));
    tangent = normalize(tangent - dot(tangent, vs_out.normal) * vs_out.normal);
    vec3 bitangent = cross(tangent, vs_out.normal);
    vs_out.TBN = mat3(tangent, bitangent, vs_out.normal);

    vs_out.fragPositionTangent = transpose(vs_out.TBN) * vs_out.fragPosition;
    vs_out.viewPosTangent = transpose(vs_out.TBN) * vs_out.viewPos;
}

#shader fragment
#version 430 core

#define LIGHTS_CAPASITY 10

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
struct PointLight {
    vec3 position;

    vec3 color;

    float attenuation;

    mat4 projectionMat;
    mat4 viewMat;

    // samplerCube depthMap;
};
struct SpotLight {
    vec3 position;
    vec3 direction;

    float innerCutoff;
    float outerCutoff;

    vec3 color;

    float attenuation;

    mat4 projectionMat;
    mat4 viewMat;
    
    // sampler2D depthMap;
};
struct DirectionalLight {
    vec3 direction;
    vec3 color;

    mat4 projectionMat;
    mat4 viewMat;
    
    // sampler2D depthMap;
};

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

uniform SpotLight        u_spotLights [LIGHTS_CAPASITY];
uniform DirectionalLight u_dirLights  [LIGHTS_CAPASITY];
uniform PointLight       u_pointLights[LIGHTS_CAPASITY];

uniform int u_spotLightCount;
uniform int u_dirLightCount;
uniform int u_pointLightCount;

out vec4 o_color;

vec4 calculateLight(PointLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords);
vec4 calculateLight(DirectionalLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords);
vec4 calculateLight(SpotLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords);
float calculateShadow(PointLight light);
float calculateShadow(DirectionalLight light);
float calculateShadow(SpotLight light);
vec2 parralaxMapping(sampler2D displacementMap, vec2 texCoords, vec3 viewDirTangent, float scale);

void main() {
    vec3 viewDir = normalize(fs_in.viewPos - fs_in.fragPosition);
    vec3 viewDirTangent = normalize(fs_in.viewPosTangent - fs_in.fragPositionTangent);

    vec2 texCoords;
    if(u_material.heightSet) {
        texCoords = parralaxMapping(u_material.height, fs_in.texCoords, viewDirTangent, 0.1);
        if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) discard;
    } else {
        texCoords = fs_in.texCoords;
    }

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

    // o_color = vec4(vec3(fs_in.fragPositionTangent), 1);
    // gamma correction moved to post process for now
    // o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
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
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), u_material.shininess) * 
        (material.specularSet ? vec3(texture(material.specular, texCoords)) : vec3(.75));
    float shadow = calculateShadow(light);

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}
vec4 calculateLight(DirectionalLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords) {
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient = light.color * 0.125;
    vec3 diffuse = 
        light.color * 
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = 
        light.color * 
        pow(max(dot(norm, normalize(lightDir + viewDir)), 0.0), u_material.shininess) * 
        (material.specularSet ? vec3(texture(material.specular, texCoords)) : vec3(.25));
    float shadow = calculateShadow(light);

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}
vec4 calculateLight(SpotLight light, Material material, vec3 norm, vec3 viewDir, vec2 texCoords) {
    
    vec3 lightDir = normalize(light.position - fs_in.fragPosition);
    float distanceLightFragment = length(light.position - fs_in.fragPosition);
    float attenuation = 1.0 / (light.attenuation * distanceLightFragment * distanceLightFragment);

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
            (material.specularSet ? vec3(texture(material.specular, texCoords)) : vec3(.25));
        float shadow = calculateShadow(light);

        return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
    } else {
        return vec4(ambient, 1.0); // unsure about that
    }
}

float calculateShadow(PointLight light) {
    return 0;
    // vec3 fragToLight = vec3(fs_in.fragPosition.xyz - light.position);
    // float currentDepth = length(fragToLight);

    // float bias = 0.05;
    // float shadow = 0.0;

    // // filtering
    // float samplesPerAxis = 4;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samplesPerAxis * 0.5))
    //     for(float y = -offset; y < offset; y += offset / (samplesPerAxis * 0.5))
    //         for(float z = -offset; z < offset; z += offset / (samplesPerAxis * 0.5)) {
    //             float closestDepth = texture(light.depthMap, fragToLight + vec3(x, y, z)).r * 100; // 100 -- far plane of the light's frustrum
    //             shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    //         }
    // shadow /= samplesPerAxis * samplesPerAxis * samplesPerAxis;
    // return shadow;
}
float calculateShadow(DirectionalLight light) {
    return 0;
    // vec4 fragPosLightSpace = light.projectionMat * light.viewMat * fs_in.fragPosition;
    // vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // projectedCoords = projectedCoords * 0.5 + 0.5; // from 0 to 1
    // if(projectedCoords.z > 0.99) return 0;
    // float currentDepth = projectedCoords.z;

    // float bias = min(-dot(normalize(light.direction) * 0.001, fs_in.normal), 0.0009);
    // float shadow = 0.0;

    // // filtering
    // vec2 texelSize = 1.0 / textureSize(light.depthMap, 0);
    // int numSamples = 0;
    // for(int x = -1; x <= 1; ++x) {
    //     for(int y = -1; y <= 1; ++y) {
    //         float closestDepth = texture(light.depthMap, projectedCoords.xy + vec2(x, y) * texelSize).r;
    //         shadow += (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    //         ++numSamples;
    //     }
    // }
    // shadow /= numSamples;
    // return shadow;
}
float calculateShadow(SpotLight light) {
    return 0; // nah
}

vec2 parralaxMapping(sampler2D displacementMap, vec2 texCoords, vec3 viewDirTangent, float scale) {
    const float minLayers = 10;
    const float maxLayers = 200;

    const float numLayers = mix(minLayers, maxLayers, dot(vec3(0, 0, 1), viewDirTangent));

    float layerDepth = 1 / numLayers;
    float currentLayerDepth = 0;
    vec2 offsetPerLayer = viewDirTangent.xy / viewDirTangent.z * scale / numLayers;

    vec2 newTexCoords = texCoords;
    float currentDepth = texture(displacementMap, newTexCoords).r;
    while(currentLayerDepth < currentDepth) {
        newTexCoords.x -= offsetPerLayer.x; // why
        newTexCoords.y += offsetPerLayer.y;
        currentDepth = texture(displacementMap, newTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = newTexCoords + offsetPerLayer;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepth - currentLayerDepth;
    float beforeDepth = texture(displacementMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + newTexCoords * (1.0 - weight);

    return finalTexCoords;
}