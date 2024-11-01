#pragma once

class IndexBuffer {
private:
    unsigned m_RenderID;
    unsigned m_size;
public:
    IndexBuffer(const GLuint *data, size_t size);
    ~IndexBuffer();
    void bind() const;
    void unbind() const;
    inline unsigned getSize() const {
        return m_size;
    }
};
