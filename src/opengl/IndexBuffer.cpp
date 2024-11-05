#include "glad/gl.h"

#include "IndexBuffer.hpp"

IndexBuffer::IndexBuffer(const GLuint *data, size_t size) : m_size(size) {
    glGenBuffers(1, &m_RenderID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
IndexBuffer::IndexBuffer() = default;
IndexBuffer::~IndexBuffer()
{
    if(m_RenderID) {
        glDeleteBuffers(1, &m_RenderID);
        m_RenderID = 0;
    }
}
void IndexBuffer::bind() const {
    if(m_RenderID) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
}
void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}