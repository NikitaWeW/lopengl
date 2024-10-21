#pragma once
#include "glad/gl.h"

class IndexBuffer {
private:
    unsigned m_RenderID;
    unsigned m_count;
public:
    IndexBuffer(const GLuint *data, size_t count);
    ~IndexBuffer();
    void bind() const;
    void unbind() const;
    inline unsigned getCount() const {
        return m_count;
    }
};
