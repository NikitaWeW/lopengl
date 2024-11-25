#include "glad/gl.h"
#include <cassert>

#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "glType.hpp"

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_RenderID);
    m_managing = true;
}
VertexArray::VertexArray(VertexArray const &other) {
    copy(other);
}
VertexArray::VertexArray(VertexArray &&other) {
    swap(std::forward<VertexArray>(other));
}
void VertexArray::operator=(VertexArray const &other) {
    copy(other);
}
void VertexArray::copy(VertexArray const &other) {
    std::swap(m_managing, other.m_managing);
    m_RenderID = other.m_RenderID;
}
void VertexArray::swap(VertexArray &&other) {
    std::swap(m_managing, other.m_managing);
    std::swap(m_RenderID, other.m_RenderID);
}
VertexArray::~VertexArray()
{
    if(m_managing) {
        glDeleteVertexArrays(1, &m_RenderID);
    }
    m_RenderID = 0;
}
void VertexArray::bind() const {
    {
        glBindVertexArray(m_RenderID);
    }
}
void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VertexArray::addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout) {
    bind();
    VB.bind();
    auto const& elements = layout.getElements(); 
    unsigned offset = 0;
    for(unsigned i = 0; i < elements.size(); ++i) {
        auto const& element = elements.at(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalised, layout.getStride(), reinterpret_cast<void const *>(offset));
        glEnableVertexAttribArray(i);
        offset += element.count * getSizeOfGLType(element.type);
    }
}
