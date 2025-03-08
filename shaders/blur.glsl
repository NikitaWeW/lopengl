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

in VS_OUT {
    vec2 texCoords;
} fs_in;

out vec4 o_color;
uniform sampler2D u_texture;

// ah yes, perfect name
const int halfNumSamplesPerAxis = 2;

void main() {
    vec2 size = textureSize(u_texture, 0);
    vec2 texelSize = 1 / size;
    vec4 result = vec4(0);
    for(int y = -halfNumSamplesPerAxis; y <= halfNumSamplesPerAxis; ++y) {
        for(int x = -halfNumSamplesPerAxis; x <= halfNumSamplesPerAxis; ++x) {
            vec2 offset = vec2(x, y) * texelSize;
            result += texture(u_texture, fs_in.texCoords + offset);
        }
    }
    result /= halfNumSamplesPerAxis*halfNumSamplesPerAxis*4; // average
    o_color = result;
}