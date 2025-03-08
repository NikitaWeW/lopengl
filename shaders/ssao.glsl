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

#define KERNEL_SIZE 64

struct Material {
    sampler2D position;
    sampler2D normal;
    sampler2D noise;
};

in VS_OUT {
    vec2 texCoords;
} fs_in;

uniform Material u_material;
uniform vec3 u_samples[KERNEL_SIZE];
uniform vec2 u_noiseScale;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

out vec4 o_occlusion;

const float radius = 0.5;
const float bias = 0.025;

void main() {
    vec4 sampleFragPositionView = u_viewMat * texture(u_material.position, fs_in.texCoords);
    vec3 fragPositionView = sampleFragPositionView.xyz;
    vec3 normal = (u_viewMat * vec4(texture(u_material.normal, fs_in.texCoords).rgb, 0)).rgb;
    vec3 randomVector = texture(u_material.noise, fs_in.texCoords * u_noiseScale).rgb;

    vec3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0;
    for(int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 samplePosition = fragPositionView + TBN * u_samples[i] * radius;
        vec4 offset = u_projectionMat * vec4(samplePosition, 1);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        float sampleDepth = (u_viewMat * texture(u_material.position, offset.xy)).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPositionView.z - sampleDepth));
        occlusion += (sampleDepth >= samplePosition.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / KERNEL_SIZE);
    o_occlusion = vec4(vec3(occlusion * sampleFragPositionView.a),1);
    // o_occlusion = vec4(randomVector*0.5+0.5,1);
}