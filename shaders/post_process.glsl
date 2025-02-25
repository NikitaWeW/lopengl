#shader vertex
#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 2) in vec2 a_texCoords;

out vec2 v_texCoords;

void main() {
    gl_Position = vec4(a_position.xy, 0, 1);
    v_texCoords = a_texCoords;
}

#shader fragment
#version 430 core
in vec2 v_texCoords;
uniform sampler2DMS u_texture;

out vec4 o_color;

vec4 textureMS(sampler2DMS sampler, vec2 coord, int samples)
{
    vec4 color = vec4(0.0);

    ivec2 size = textureSize(sampler);

    for (int i = 0; i < samples; i++)
        color += texelFetch(sampler, ivec2(size * coord), i);

    color /= float(samples);

    return color;
}

void main() {
    vec3 hdrColor = textureMS(u_texture, (v_texCoords), 4).rgb;
    vec3 mappedColor = hdrColor / (1 + hdrColor);
    o_color = vec4(mappedColor, 1);
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}