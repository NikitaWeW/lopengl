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
#define RADIUS 0.5
#define BIAS 0.025

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

out float o_occlusion;

void main() {
    vec3 fragPositionView = u_viewMat * texture(u_material.position, fs_in.texCoords).rgb;
    vec3 normal = texture(u_material.normal, fs_in.texCoords).rgb;
    vec3 randomVector = texture(u_material.noise, fs_in.texCoords * u_noiseScale).rgb;

    vec3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0;
    for(int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 sample = fragPositionView + TBN * u_samples[i] * RADIUS;
        vec4 offset = u_projectionMat * vec4(sample, 1);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        float sampleDepth = (u_viewMat * texture(u_material.position, offset.xy)).z;
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPositionView.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + BIAS ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    o_occlusion = occlusion;
}