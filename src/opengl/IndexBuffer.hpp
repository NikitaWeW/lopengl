#pragma once
#include "glad/gl.h"

class IndexBuffer {
private:
    unsigned m_RenderID = 0;
    unsigned m_size = 0;
    mutable bool m_managing = true;
public:
    IndexBuffer(const GLuint *data, size_t size);
    IndexBuffer();
    ~IndexBuffer();
    IndexBuffer(IndexBuffer const &other);
    IndexBuffer(IndexBuffer &&other);
    void operator=(IndexBuffer const &other);
    void copy(IndexBuffer const &other);
    void swap(IndexBuffer &&other);
    void bind() const;
    void unbind() const;
    inline unsigned getSize() const {
        return m_size;
    }
};
