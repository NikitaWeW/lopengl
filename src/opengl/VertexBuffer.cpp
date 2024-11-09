#include "glad/gl.h"

#include "VertexBuffer.hpp"
#include "Renderer.hpp"

VertexBufferLayout::VertexBufferLayout() : m_stride(0) {}
VertexBufferLayout::~VertexBufferLayout() = default;
void VertexBufferLayout::push(unsigned const count, unsigned type, bool normalised) {
    m_elements.push_back({type, count, normalised});
    m_stride += getSizeOfGLType(type) * count;
}

VertexBuffer::VertexBuffer(const void *data, size_t size) {
    glGenBuffers(1, &m_RenderID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
VertexBuffer::VertexBuffer() = default;
VertexBuffer::~VertexBuffer()
{
    if(m_managing) {
        glDeleteBuffers(1, &m_RenderID);
        m_RenderID = 0;
    }
}
VertexBuffer::VertexBuffer(VertexBuffer const &other) {
    copy(other);
}
VertexBuffer::VertexBuffer(VertexBuffer &&other) {
    swap(std::forward<VertexBuffer>(other));
}
void VertexBuffer::operator=(VertexBuffer const &other) {
    copy(other);
}
void VertexBuffer::copy(VertexBuffer const &other) {
    std::swap(m_managing, other.m_managing);
    m_RenderID = other.m_RenderID;
}
void VertexBuffer::swap(VertexBuffer &&other) {
    std::swap(m_managing, other.m_managing);
    std::swap(m_RenderID, other.m_RenderID);
}
void VertexBuffer::bind() const
{
    if(m_RenderID) glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
}
void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}