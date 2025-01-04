#include "glad/gl.h"
#include <utility>
#include "IndexBuffer.hpp"

void IndexBuffer::bind() const   { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderID); }
void IndexBuffer::unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

IndexBuffer::IndexBuffer(const GLuint *data, size_t size) : m_size(size)
{
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
IndexBuffer::IndexBuffer(size_t size) : m_size(size)
{
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

IndexBuffer::IndexBuffer() = default;
IndexBuffer::~IndexBuffer() { if(canDeallocate()) glDeleteBuffers(1, &m_renderID); }
