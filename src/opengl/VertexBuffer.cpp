#include "glad/gl.h"

#include "VertexBuffer.hpp"
#include "Renderer.hpp"

VertexBufferlayout::VertexBufferlayout() : m_stride(0) {}
VertexBufferlayout::~VertexBufferlayout() = default;
template <typename T>
void VertexBufferlayout::push(unsigned const count) {
    static_assert(false && "type not supported");
}
template <>
void VertexBufferlayout::push<float>(unsigned const count) {
    m_elements.push_back({GL_FLOAT, count, true});
    m_stride += getSizeOfGLType(GL_FLOAT) * count;
}
template <>
void VertexBufferlayout::push<unsigned>(unsigned const count) {
    m_elements.push_back({GL_UNSIGNED_INT, count, false});
    m_stride += getSizeOfGLType(GL_UNSIGNED_INT) * count;
}
template <>
void VertexBufferlayout::push<unsigned char>(unsigned const count) {
    m_elements.push_back({GL_UNSIGNED_BYTE, count, true});
    m_stride += getSizeOfGLType(GL_UNSIGNED_BYTE) * count;
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