#include "ShaderStorage.hpp"
#include "glad/gl.h"

ogl::UniformBuffer::UniformBuffer(int) noexcept
{
    glGenBuffers(1, &m_renderID);
}

ogl::UniformBuffer::~UniformBuffer()
{
    if(canDeallocate()) {
        glDeleteBuffers(1, &m_renderID);
    }
}

void ogl::UniformBuffer::bind(unsigned slot) const noexcept { glBindBuffer(GL_UNIFORM_BUFFER, m_renderID); }
void ogl::UniformBuffer::bindingPoint(unsigned index) const noexcept { glBindBufferBase(GL_UNIFORM_BUFFER, index, m_renderID); }

ogl::SSBO::SSBO(int) noexcept
{
    glCreateBuffers(1, &m_renderID);
}

ogl::SSBO::~SSBO()
{
    if(canDeallocate()) {
        glDeleteBuffers(1, &m_renderID);
    }
}

void ogl::SSBO::bind(unsigned slot) const noexcept { glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_renderID); }
void ogl::SSBO::bindingPoint(unsigned index) const noexcept { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_renderID); }
