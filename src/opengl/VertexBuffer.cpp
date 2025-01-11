#include "glad/gl.h"

#include "VertexBuffer.hpp"
#include "glType.hpp"

void InterleavedVertexBufferLayout::push(unsigned const count, unsigned type) {
    m_elements.push_back({count, type});
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

VertexBufferLayout::VertexBufferLayout(std::vector<Element> const &elements) {
    for(Element const &element : elements) {
        push(element.count, element.type, element.offset);
    }
}
InterleavedVertexBufferLayout::InterleavedVertexBufferLayout(std::vector<Element> const &elements) {
    for(Element const &element : elements) {
        push(element.count, element.type);
    }
}

InterleavedVertexBufferLayout::InterleavedVertexBufferLayout(std::initializer_list<Element> const &elements) : InterleavedVertexBufferLayout(std::vector(elements))
{
}
VertexBufferLayout::VertexBufferLayout(std::initializer_list<Element> const &elements) : VertexBufferLayout(std::vector(elements))
{
}

void VertexBufferLayout::push(unsigned const count, unsigned type, size_t offset)
{
    m_elements.push_back({count, type, offset});
}

InstancedArrayLayout::InstancedArrayLayout(std::vector<Element> const &elements)
{
    for(Element const &element : elements) {
        push(element.count, element.type, element.divisor);
    }
}

InstancedArrayLayout::InstancedArrayLayout(std::initializer_list<Element> const &elements) : InstancedArrayLayout(std::vector(elements))
{
}

void InstancedArrayLayout::push(unsigned const count, unsigned type, unsigned divisor)
{
    m_elements.push_back({count, type, divisor});
    m_stride += count * getSizeOfGLType(type);
}
