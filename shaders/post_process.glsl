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

void main() {

    o_color = texture(u_texture, v_texCoords);

//  ========================
//      kernel effects    //
//  ========================
    // const float offset = 1 / 300;
    // vec2 const offsets[9] = vec2[](
    //     vec2(-offset, offset), // top-left
    //     vec2( 0.0f, offset), // top-center
    //     vec2( offset, offset), // top-right
    //     vec2(-offset, 0.0f), // center-left
    //     vec2( 0.0f, 0.0f), // center-center
    //     vec2( offset, 0.0f), // center-right
    //     vec2(-offset, -offset), // bottom-left
    //     vec2( 0.0f, -offset), // bottom-center
    //     vec2( offset, -offset) // bottom-right
    // );
    // float kernel[9] = float[](
    //     -1, -1, -1,
    //     -1,  9, -1,
    //     -1, -1, -1
    // );

//  =================================
//      post processing effects    //
//  =================================
    // inversion
    // o_color = vec4(vec3(1) - o_color.rgb, 1); 
    // grayscale
    // o_color = vec4(vec3(0.21 * o_color.r + 0.71 * o_color.g + 0.72 * o_color.b), 1);

}