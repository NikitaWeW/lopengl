#include "glad/gl.h"
#include <cassert>

#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "glType.hpp"

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_RenderID);
}

VertexArray::~VertexArray()
{
    if(canDeallocate()) {
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

void VertexArray::addBuffer(VertexBuffer const &VB, InterleavedVertexBufferLayout const &layout) {
    bind();
    VB.bind();
    auto const &elements = layout.getElements(); 
    unsigned offset = 0;
    for(InterleavedVertexBufferLayout::Element const &element : elements) {
        glVertexAttribPointer(m_vertexAttribIndex, element.count, element.type, false, layout.getStride(), reinterpret_cast<void const *>(offset));
        glEnableVertexAttribArray(m_vertexAttribIndex);
        offset += element.count * getSizeOfGLType(element.type);
        ++m_vertexAttribIndex;
    }
}

void VertexArray::addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout) {
    bind();
    VB.bind();
    auto const &elements = layout.getElements(); 
    for(VertexBufferLayout::Element const &element : elements) {
        glVertexAttribPointer(m_vertexAttribIndex, element.count, element.type, false, element.count * getSizeOfGLType(element.type), reinterpret_cast<void const *>(element.offset));
        glEnableVertexAttribArray(m_vertexAttribIndex);
        ++m_vertexAttribIndex;
    }
}

void VertexArray::addBuffer(VertexBuffer const &VB, InstancedArrayLayout const &layout) {
    bind();
    VB.bind();
    auto const &elements = layout.getElements(); 
    unsigned offset = 0;
    for(InstancedArrayLayout::Element const &element : elements) {
        glVertexAttribPointer(m_vertexAttribIndex, element.count, element.type, false, layout.getStride(), reinterpret_cast<void const *>(offset));
        glVertexAttribDivisor(m_vertexAttribIndex, element.divisor);
        glEnableVertexAttribArray(m_vertexAttribIndex);
        offset += element.count * getSizeOfGLType(element.type);
        ++m_vertexAttribIndex;
    }
}
