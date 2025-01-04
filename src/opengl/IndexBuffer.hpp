#pragma once
#include <cstddef>
#include "glad/gl.h"
#include "utils/Resource.hpp"

class IndexBuffer : public Resource {
private:
    unsigned m_size = 0;
    unsigned m_renderID = 0;
public:
    void bind() const;
    void unbind() const;

    IndexBuffer(const GLuint *data, size_t size);
    IndexBuffer(size_t size);
    IndexBuffer();
    ~IndexBuffer();
    
    inline size_t getSize() const { return m_size; }
};
