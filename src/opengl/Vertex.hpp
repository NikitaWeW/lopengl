#pragma once
#include "VertexBuffer.hpp"
#include "glad/gl.h"
#include "glm/glm.hpp"

struct Vertex {
    glm::vec3 position = glm::vec3{0};
    glm::vec3 normals = glm::vec3{0};
    glm::vec2 textureCoords = glm::vec2{0};
    // float textureIndex = 0; // i know this is stupid, but opengl does not like anythyng else
};

inline VertexBufferLayout getVertexLayout() {
    VertexBufferLayout layout;
    layout.push(3, GL_FLOAT);
    layout.push(3, GL_FLOAT);
    layout.push(2, GL_FLOAT);
    return layout;
}