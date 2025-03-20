#include "ShaderStorageBuffer.hpp"
#include "glad/gl.h"

SSBO::SSBO() = default;

SSBO::SSBO(size_t size)
{
    glCreateBuffers(1, &m_renderID);
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

void SSBO::bind(unsigned slot) const   { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_renderID); }
void SSBO::unbind(unsigned slot) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0); }
