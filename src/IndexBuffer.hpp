#pragma once
#include <glad/gl.h>

class IndexBuffer {
private:
    unsigned m_RenderID;
public:
    IndexBuffer(const GLuint *data, size_t count);
    ~IndexBuffer();
    void bind() const;
    void unbind() const;
};
