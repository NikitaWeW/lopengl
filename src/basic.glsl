#shader vertex
#version 330
layout (location = 0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 330
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.29, 0.59, 0.5, 1.0);
}