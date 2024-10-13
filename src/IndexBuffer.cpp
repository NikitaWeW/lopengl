#include <glad/gl.h>

#include "Renderer.hpp"
#include "IndexBuffer.hpp"

IndexBuffer::IndexBuffer(const GLuint *data, size_t count) {
    GLCALL(glGenBuffers(1, &m_RenderID));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), data, GL_STATIC_DRAW));
}
IndexBuffer::~IndexBuffer() {
    GLCALL(glDeleteBuffers(1, &m_RenderID));
}
void IndexBuffer::bind() const {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID));
}
void IndexBuffer::unbind() const {
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}