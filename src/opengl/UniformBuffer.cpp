#include "UniformBuffer.hpp"
#include "glad/gl.h"

UniformBuffer::UniformBuffer(size_t size)
{
    glGenBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

UniformBuffer::~UniformBuffer()
{
    if(canDeallocate()) glDeleteBuffers(1, &m_renderID);
}

void UniformBuffer::bind() const   { glBindBuffer(GL_UNIFORM_BUFFER, m_renderID); }
void UniformBuffer::unbind() const { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

void UniformBuffer::bindingPoint(unsigned index) const { glBindBufferBase(GL_UNIFORM_BUFFER, index, m_renderID); }
