#shader vertex
#version 430 core
layout (location = 0) in vec3 a_position;
// normals at location = 1
layout (location = 2) in vec2 a_texCoords;

out vec2 v_texCoords;

void main() {
    gl_Position = vec4(a_position.xy, 0, 1);
    v_texCoords = a_texCoords;
}

#shader fragment
#version 430 core
in vec2 v_texCoords;
uniform sampler2D u_texture;

out vec4 o_color;

void main()
{
    o_color = texture(u_texture, v_texCoords);
//  ======================================== //
//              kernel effects               //
//  ======================================== //
    // input here
    const float offset = 1.0 / 300.0;
    // float kernel[9] = float[]( // blur kernel
    //     1.0 / 16, 2.0 / 16, 1.0 / 16,
    //     2.0 / 16, 4.0 / 16, 2.0 / 16,
    //     1.0 / 16, 2.0 / 16, 1.0 / 16
    // );
    // float kernel[9] = float[]( // sharpen kernel
    //     -1,-1,-1,
    //     -1, 9,-1,
    //     -1,-1,-1
    // );
    // float kernel[9] = float[]( // edge detection kernel
    //     1, 1, 1,
    //     1,-8, 1,
    //     1, 1, 1
    // );

//  ========================================
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // top-left
        vec2( 0.0f, offset), // top-center
        vec2( offset, offset), // top-right
        vec2(-offset, 0.0f), // center-left
        vec2( 0.0f, 0.0f), // center-center
        vec2( offset, 0.0f), // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f, -offset), // bottom-center
        vec2( offset, -offset) // bottom-right
    );
    vec3 samples[9];
    for(int i = 0; i < 9; i++)
        samples[i] = vec3(texture(u_texture, v_texCoords.st + offsets[i]));
    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += samples[i] * kernel[i];
    o_color = vec4(color, 1.0);
}