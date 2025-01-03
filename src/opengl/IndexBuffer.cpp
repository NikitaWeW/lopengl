#include "glad/gl.h"
#include <utility>
#include "IndexBuffer.hpp"

void IndexBuffer::bufferData(const void *data, size_t size) const             { glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW); }
void IndexBuffer::subdata(size_t offset, size_t size, void const *data) const { glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data); }
void IndexBuffer::allocate(size_t size) const                                 { glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW); }
void IndexBuffer::deallocate() const                                          { glDeleteBuffers(1, &m_renderID); }
bool IndexBuffer::unmap() const                                { bind(); return glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); }
void IndexBuffer::bind() const                                                { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderID); }
void IndexBuffer::unbind() const                                              { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
void *IndexBuffer::map(GLenum access) const {                    bind(); return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, access); }


IndexBuffer::IndexBuffer(const GLuint *data, size_t size) : m_size(size)
{
    glGenBuffers(1, &m_renderID);
    bind();
    bufferData(data, size);
}
IndexBuffer::IndexBuffer(size_t size) : m_size(size)
{
    glGenBuffers(1, &m_renderID);
    bind();
    allocate(size);
}

IndexBuffer::IndexBuffer() = default;
IndexBuffer::~IndexBuffer() { if(canDeallocate()) deallocate(); }
