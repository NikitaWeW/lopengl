#pragma once
#include <stddef.h>
#include <vector>
#include "Buffer.hpp"

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
    inline unsigned getStride() const { return m_stride; }
    inline std::vector<VertexBufferlayoutElement> const &getElements() const { return m_elements; }
};
class VertexBuffer : public Buffer {
public:
    VertexBuffer(const void *data, size_t size);
    VertexBuffer();
    ~VertexBuffer();

    void bufferData(void const *data, size_t size) const;
    void subdata(size_t offset, size_t size, void const *data) const;
    void allocate(size_t size) const;
    void deallocate() const;
    void*map(unsigned access) const;
    bool unmap() const;
    void bind() const;
    void unbind() const;
};