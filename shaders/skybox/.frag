#version 430 core

in vec3 v_texCoords;
layout (binding = 0) uniform samplerCube u_skybox;
layout (depth_greater) out float gl_FragDepth;
out vec4 o_color;

// float linearizeDepth(float depth, float near_plane, float far_plane) { return (2.0 * near_plane * far_plane) / (far_plane + near_plane - (depth * 2.0 - 1.0) * (far_plane - near_plane)); }

void main() {
    o_color = texture(u_skybox, v_texCoords);
    // o_color = vec4(0.1 * vec3(linearizeDepth(texture(u_skybox, v_texCoords).r, 0.01, 100)), 1); // omni-directional shadow depth cubemap debug
    gl_FragDepth = 1.0;
}