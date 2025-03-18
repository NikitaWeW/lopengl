#shader vertex
#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texCoords; // !

out vec2 v_texCoords;

void main() {
    gl_Position = vec4(a_position.xy, 0, 1);
    v_texCoords = a_texCoords;
}

#shader fragment
#version 430 core
in vec2 v_texCoords;

uniform float u_exposure = 1;
uniform sampler2D u_texture;

out vec4 o_color;

void main() {
    vec3 hdrColor = texture(u_texture, v_texCoords).rgb;

    vec3 mappedColor = 1 - exp(-hdrColor * u_exposure);
    
    o_color = vec4(mappedColor, 1);
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}