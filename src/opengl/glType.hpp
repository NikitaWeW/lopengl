#pragma once
#include "glad/gl.h"
#include <cstddef>
#include <cassert>

inline size_t getSizeOfGLType(unsigned type) {
    switch (type) {
        case GL_BYTE:            return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE:   return sizeof(GLubyte);
        case GL_SHORT:           return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:  return sizeof(GLushort);
        case GL_INT:             return sizeof(GLint);
        case GL_UNSIGNED_INT:    return sizeof(GLuint);
        case GL_FLOAT:           return sizeof(GLfloat);
        case GL_DOUBLE:          return sizeof(GLdouble);
        default: 
            assert(false && "type not supported");
            return 0;
    }
}