#shader vertex
#version 330 core
layout (location = 0) in vec4 a_position;
layout (location = 2) in vec2 a_texCoords;

out vec2 v_texCoords;

void main() {
    gl_Position = a_position;
    v_texCoords = a_texCoords;
}

#shader fragment
#version 330 core
in vec2 v_texCoords;

uniform sampler2D u_texture;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
uniform bool u_horizontal;

out vec4 o_color;

void main() {
    vec2 texelSize = 1.0 / textureSize(u_texture, 0);
    vec3 result = texture(u_texture, v_texCoords).rgb * weight[0];

    if(u_horizontal) {
        for(int i = 1; i < 5; ++i) {
            result += texture(u_texture, v_texCoords + vec2(texelSize.x * i, 0)).rgb * weight[i];
            result += texture(u_texture, v_texCoords - vec2(texelSize.x * i, 0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i) {
            result += texture(u_texture, v_texCoords + vec2(0, texelSize.y * i)).rgb * weight[i];
            result += texture(u_texture, v_texCoords - vec2(0, texelSize.y * i)).rgb * weight[i];
        }
    }

    o_color = vec4(result, 1);
}