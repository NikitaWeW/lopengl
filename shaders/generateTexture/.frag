#version 430

in vec2 v_texCoord;
out vec3 o_color;

uniform vec2 u_dimensions;
uniform uint u_face;

vec3 getDir() {
    // u_face is face
    vec3 dir = vec3(0.0, 0.0, 0.0);
    vec2 uv = v_texCoord * 2 - 1;

    // 0  GL_TEXTURE_CUBE_MAP_POSITIVE_X
    if (u_face == 0) {
    dir = vec3(1.0, -uv.y, -uv.x);
    }
    // 1  GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    else if (u_face == 1) {
    dir = vec3(-1.0, -uv.y, uv.x);
    }
    // 2  GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    else if (u_face == 2) {
    dir = vec3(uv.x, 1.0, uv.y);
    }
    // 3  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    else if (u_face == 3) {
    dir = vec3(uv.x, -1.0, -uv.y);
    }
    // 4  GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    else if (u_face == 4) {
    dir = vec3(uv.x, -uv.y, 1.0);
    }
    // 5  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    else {
    dir = vec3(-uv.x, -uv.y, -1.0);
    }
    return normalize(dir);
} 

void main() 
{
    o_color.rgb = vec3(getDir());
}