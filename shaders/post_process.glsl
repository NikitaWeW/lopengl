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
uniform sampler2D u_texture;

out vec4 o_color;

float LinearizeDepth(float depth)
{
    float near_plane = 0.1;
    float far_plane = 100.0;
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    o_color = texture(u_texture, v_texCoords);
    // o_color = vec4(vec3(0.2126 * o_color.r + 0.7152 * o_color.g + 0.0722 * o_color.b), 1.0); // grayscale
    o_color = vec4(vec3(LinearizeDepth(o_color.r)), 1);
}