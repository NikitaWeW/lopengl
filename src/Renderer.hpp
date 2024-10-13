#pragma once
#include <logger.h>

#define GLCALL(x) \
    x; \
    while (GLenum GLerror = glGetError()) { \
        char const *errorString;\
        switch (GLerror) {\
            case GL_NO_ERROR:\
                errorString = "No error";\
                break;\
            case GL_INVALID_ENUM:\
                errorString = "Invalid enum";\
                break;\
            case GL_INVALID_VALUE:\
                errorString = "Invalid value";\
                break;\
            case GL_INVALID_OPERATION:\
                errorString = "Invalid operation";\
                break;\
            case GL_STACK_OVERFLOW:\
                errorString = "Stack overflow";\
                break;\
            case GL_STACK_UNDERFLOW:\
                errorString = "Stack underflow";\
                break;\
            case GL_OUT_OF_MEMORY:\
                errorString = "Out of memory";\
                break;\
            case GL_INVALID_FRAMEBUFFER_OPERATION:\
                errorString = "Invalid framebuffer operation";\
                break;\
            default:\
                errorString = "Unknown error";\
                break;\
            }\
        LOG_ERROR("opengl error: %s", errorString); \
    } 


size_t getSizeOfGLType(unsigned type);