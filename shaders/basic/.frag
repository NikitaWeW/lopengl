#version 430

out vec4 oColor;

in VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    flat mat3 TBN;
} fs_in;

layout(binding = 0) uniform sampler2D uAlbedo;
layout(binding = 1) uniform sampler2D uMetallic;
layout(binding = 2) uniform sampler2D uRoughness;
layout(binding = 3) uniform sampler2D uAmbient;
layout(binding = 4) uniform sampler2D uNormal;
layout(binding = 5) uniform sampler2D uDisplacement;

void main()
{
    oColor = texture(uAlbedo, fs_in.texCoords);
}