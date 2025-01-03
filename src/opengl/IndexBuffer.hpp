#pragma once
#include <cstddef>
#include "glad/gl.h"
#include "Buffer.hpp"

class IndexBuffer : public Buffer {
private:
    unsigned m_size = 0;
public:
    void bufferData(void const *data, size_t size) const;
    void subdata(size_t offset, size_t size, void const *data) const;
    void allocate(size_t size) const;
    void deallocate() const;
    void *map(GLenum access = GL_READ_WRITE) const;
    bool unmap() const;
    void bind() const;
    void unbind() const;

    IndexBuffer(const GLuint *data, size_t size);
    IndexBuffer(size_t size);
    IndexBuffer();
    ~IndexBuffer();
    
    inline size_t getSize() const { return m_size; }
};
