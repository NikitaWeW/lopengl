#version 430

layout(binding = 0) uniform sampler2DMS u_texture;
uniform float u_exposure = 1;

in vec2 v_texCoord;
out vec4 o_color;

vec4 texture(sampler2DMS tex, vec2 texCoord, int numSamples)
{
    vec4 color = vec4(0);
    ivec2 size = textureSize(tex);

    for(int i = 0; i < numSamples; ++i)
    {
        color += texelFetch(tex, ivec2(vec2(size) * texCoord), i);
    }

    color /= numSamples;

    return color;
}

void main() 
{
    vec3 hdrColor = texture(u_texture, v_texCoord, 4).rgb;
    vec3 mappedColor = 1 - exp(-hdrColor * u_exposure);
    
    o_color = vec4(mappedColor, 1);
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}