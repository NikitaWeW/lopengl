#pragma once

class IndexBuffer {
private:
    unsigned m_RenderID = 0;
    unsigned m_size = 0;
public:
    IndexBuffer(const GLuint *data, size_t size);
    IndexBuffer();
    ~IndexBuffer();
    void bind() const;
    void unbind() const;
    inline unsigned getSize() const {
        return m_size;
    }
};
