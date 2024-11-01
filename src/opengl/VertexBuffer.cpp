#include "glad/gl.h"

#include "VertexBuffer.hpp"
#include "Renderer.hpp"

VertexBufferlayout::VertexBufferlayout() : m_stride(0) {}
VertexBufferlayout::~VertexBufferlayout() = default;
void VertexBufferlayout::push(unsigned const count, unsigned type, bool normalised) {
    m_elements.push_back({type, count, normalised});
    m_stride += getSizeOfGLType(type) * count;
}

VertexBuffer::VertexBuffer(const void *data, size_t size) {
    glGenBuffers(1, &m_RenderID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_RenderID);
}
void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
}
void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}