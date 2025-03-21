#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 2) in vec2 a_texCoord;

out vec2 v_texCoord;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_texCoord = a_texCoord;
}

#shader fragment
#version 430 core
out vec4 o_color;

in vec2 v_texCoord;

layout (std140) uniform vectors {
    vec4 values[200];
    uint count;
};

void main() {
    o_color = vec4(vec3(values[1].x,values[1].z, float(count) / 3.0), 1.0);
    // o_color = vec4(values[1]);
}