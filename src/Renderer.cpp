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