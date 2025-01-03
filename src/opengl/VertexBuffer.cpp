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
    bufferData(data, size);
}
VertexBuffer::VertexBuffer() = default;
VertexBuffer::~VertexBuffer()
{
    if(canDeallocate()) deallocate();
}
void VertexBuffer::bufferData(void const *data, size_t size) const             { glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW); }
void VertexBuffer::subdata(size_t offset, size_t size, void const *data) const { glBufferSubData(GL_ARRAY_BUFFER, offset, size, data); }
void VertexBuffer::allocate(size_t size) const                                 { glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW); }
void VertexBuffer::deallocate() const                                          { glDeleteBuffers(1, &m_renderID); }
void *VertexBuffer::map(unsigned access) const                  { bind(); return glMapBuffer(GL_ARRAY_BUFFER, access); }
bool VertexBuffer::unmap() const                                { bind(); return glUnmapBuffer(GL_ARRAY_BUFFER); }
void VertexBuffer::bind() const                                                { glBindBuffer(GL_ARRAY_BUFFER, m_renderID); }
void VertexBuffer::unbind() const                                              { glBindBuffer(GL_ARRAY_BUFFER, 0); }