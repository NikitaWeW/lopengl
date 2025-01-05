#pragma once
#include "opengl/VertexBuffer.hpp"
#include "glad/gl.h"
#include "glm/glm.hpp"

struct Vertex {
    glm::vec3 position = glm::vec3{0};
    glm::vec3 normals = glm::vec3{0};
    glm::vec2 textureCoords = glm::vec2{0};
};

inline InterleavedVertexBufferLayout getVertexLayout() {
    InterleavedVertexBufferLayout layout;
    layout.push(3, GL_FLOAT);
    layout.push(3, GL_FLOAT);
    layout.push(2, GL_FLOAT);
    return layout;
}