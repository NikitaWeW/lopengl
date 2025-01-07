#include "glad/gl.h"

#include "VertexBuffer.hpp"
#include "glType.hpp"

void InterleavedVertexBufferLayout::push(unsigned const count, unsigned type) {
    m_elements.push_back({type, count});
    m_stride += count * getSizeOfGLType(type);
}

VertexBuffer::VertexBuffer(const void *data, size_t size) {
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
VertexBuffer::VertexBuffer(size_t size) {
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}
VertexBuffer::VertexBuffer() = default;
VertexBuffer::~VertexBuffer()
{
    if(canDeallocate()) glDeleteBuffers(1, &m_renderID);
}
void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_renderID); }
void VertexBuffer::unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void VertexBufferLayout::push(unsigned const count, unsigned type, size_t offset)
{
    m_elements.push_back({type, count, offset});
}
