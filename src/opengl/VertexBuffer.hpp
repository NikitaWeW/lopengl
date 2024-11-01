#pragma once
#include <stddef.h>
#include <vector>

struct VertexBufferlayoutElement {
    unsigned type;
    unsigned count;
    bool normalised;
};

class VertexBufferlayout {
private:
    std::vector<VertexBufferlayoutElement> m_elements;
    unsigned m_stride;
public:
    VertexBufferlayout();
    ~VertexBufferlayout();
    void push(unsigned const count, unsigned type, bool normalised = false);
    inline unsigned getStride() const {
        return m_stride;
    }
    inline std::vector<VertexBufferlayoutElement> const &getElements() const {
        return m_elements;
    }
};
class VertexBuffer {
private:
    unsigned m_RenderID;
public:
    VertexBuffer(const void *data, size_t size);
    ~VertexBuffer();
    void bind() const;
    void unbind() const;
};