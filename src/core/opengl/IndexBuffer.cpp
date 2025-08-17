#include "IndexBuffer.hpp"

ogl::IndexBuffer::IndexBuffer(size_t size, GLenum usage) noexcept
{
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, usage);
}

ogl::IndexBuffer::IndexBuffer(size_t size, void const *data, GLenum usage) noexcept
{
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

ogl::IndexBuffer::~IndexBuffer()
{
    if(canDeallocate())
        glDeleteBuffers(1, &m_renderID);
}

void ogl::IndexBuffer::bind(unsigned slot) const noexcept { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderID); }