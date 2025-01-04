#include "glad/gl.h"

#include "VertexBuffer.hpp"
#include "glType.hpp"

VertexBufferLayout::VertexBufferLayout() : m_stride(0) {}
VertexBufferLayout::~VertexBufferLayout() = default;
void VertexBufferLayout::push(unsigned const count, unsigned type, bool normalised) {
    m_elements.push_back({type, count, normalised});
    m_stride += getSizeOfGLType(type) * count;
}

VertexBuffer::VertexBuffer(const void *data, size_t size) {
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
VertexBuffer::VertexBuffer() = default;
VertexBuffer::~VertexBuffer()
{
    if(canDeallocate()) glDeleteBuffers(1, &m_renderID);
}
void VertexBuffer::bind() const                                                { glBindBuffer(GL_ARRAY_BUFFER, m_renderID); }
void VertexBuffer::unbind() const                                              { glBindBuffer(GL_ARRAY_BUFFER, 0); }