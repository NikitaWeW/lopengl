#version 330 core

// use glDrawArrays(GL_TRIANGLE_STRIP, 0, 14) to draw 14 vertices
// https://stackoverflow.com/a/46016469
const vec3 cubePositions[14] = vec3[](
    vec3(-1, 1, 1), // Front-top-left
    vec3( 1, 1, 1), // Front-top-right
    vec3(-1,-1, 1), // Front-bottom-left
    vec3( 1,-1, 1), // Front-bottom-right
    vec3( 1,-1,-1), // Back-bottom-right
    vec3( 1, 1, 1), // Front-top-right
    vec3( 1, 1,-1), // Back-top-right
    vec3(-1, 1, 1), // Front-top-left
    vec3(-1, 1,-1), // Back-top-left
    vec3(-1,-1, 1), // Front-bottom-left
    vec3(-1,-1,-1), // Back-bottom-left
    vec3( 1,-1,-1), // Back-bottom-right
    vec3(-1, 1,-1), // Back-top-left
    vec3( 1, 1,-1)  // Back-top-right
);

out vec3 vPos;

uniform mat4 uProjMat;
uniform mat4 uViewMat;

void main() {
    vPos = cubePositions[gl_VertexID];
    gl_Position = (uProjMat * vec4(mat3(uViewMat) * vPos, 1)).xyww;
}
