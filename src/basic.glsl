#shader vertex
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform mat4 u_MVP;

void main() {
    gl_Position = vec4(position, 1.0) * u_MVP;
    v_texCoord = texCoord;
}

#shader fragment
#version 330 core
out vec4 color;

in vec2 v_texCoord;

uniform vec4 u_color1;
uniform vec4 u_color2;
uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_texCoord);
    color = texColor * u_color2 + u_color1;
}