#include "glad/gl.h"
#include <utility>
#include "IndexBuffer.hpp"

IndexBuffer::IndexBuffer(const GLuint *data, size_t size) : m_size(size) {
    glGenBuffers(1, &m_RenderID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    m_managing = true;
}
IndexBuffer::IndexBuffer() = default;
IndexBuffer::~IndexBuffer()
{
    if(m_managing) {
        glDeleteBuffers(1, &m_RenderID);
        m_RenderID = 0;
    }
}
IndexBuffer::IndexBuffer(IndexBuffer const &other) {
    copy(other);
}
IndexBuffer::IndexBuffer(IndexBuffer &&other) {
    swap(std::forward<IndexBuffer>(other));
}
void IndexBuffer::operator=(IndexBuffer const &other) {
    copy(other);
}
void IndexBuffer::copy(IndexBuffer const &other) {
    std::swap(m_managing, other.m_managing);
    m_RenderID = other.m_RenderID;
    m_size = other.m_size;
}
void IndexBuffer::swap(IndexBuffer &&other) {
    std::swap(m_managing, other.m_managing);
    std::swap(m_RenderID, other.m_RenderID);
    std::swap(m_size, other.m_size);
}
void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
}
void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}