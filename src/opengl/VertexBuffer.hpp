#pragma once
#include <stddef.h>
#include <vector>
#include "utils/Resource.hpp"

struct VertexBufferlayoutElement {
    unsigned type;
    unsigned count;
    bool normalised;
};

class VertexBufferLayout {
private:
    std::vector<VertexBufferlayoutElement> m_elements;
    unsigned m_stride;
public:
    VertexBufferLayout();
    ~VertexBufferLayout();
    void push(unsigned const count, unsigned type, bool normalised = false);
    inline unsigned getStride() const {
        return m_stride;
    }
    inline std::vector<VertexBufferlayoutElement> const &getElements() const {
        return m_elements;
    }
};
class VertexBuffer : public Resource {
private:
    unsigned m_RenderID = 0;
public:
    VertexBuffer(const void *data, size_t size);
    VertexBuffer();
    ~VertexBuffer();
    void bind() const;
    void unbind() const;
};