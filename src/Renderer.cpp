#include <glad/gl.h>
#include <cassert>

#include "Renderer.hpp"

size_t getSizeOfGLType(unsigned type) {
    switch (type) {
        case GL_FLOAT: return sizeof(GLfloat);
        case GL_UNSIGNED_INT: return sizeof(GLuint);
        case GL_UNSIGNED_BYTE: return sizeof(GLbyte);
        default: 
            assert(false && "type not supported");
            return 0;
    }
}

void Renderer::Clear(float r, float g, float b) const {
    GLCALL(glClearColor(r, g, b, 0));
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(VertexArray const &va, IndexBuffer const &ib, Shader const &shader) const
{
    shader.bind();
    ib.bind();
    va.bind();
    GLCALL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));
}
