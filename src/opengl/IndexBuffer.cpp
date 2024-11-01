#include "glad/gl.h"

#include "IndexBuffer.hpp"

IndexBuffer::IndexBuffer(const GLuint *data, size_t size) : m_size(size) {
    glGenBuffers(1, &m_RenderID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &m_RenderID);
}
void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
}
void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}