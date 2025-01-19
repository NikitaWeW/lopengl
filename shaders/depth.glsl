#shader vertex
#version 330
layout(location = 0) in vec4 a_position;

uniform mat4 u_modelMat;

void main() {
    gl_Position = u_modelMat * a_position;
}
#shader geometry
#version 330
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 u_shadowMatrices[6];
out vec4 fragPos;

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face;
        fragPos = gl_in[face].gl_Position;
        gl_Position = u_shadowMatrices[face] * fragPos;
        EmitVertex();
    }
    EndPrimitive();
}

#shader fragment
#version 330
in vec4 fragPos;

struct PointLight {
    vec3 position;
};
uniform PointLight u_light;

void main() {
    gl_FragDepth = length(fragPos.xyz - u_light.position) / 100;
}