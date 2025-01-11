#pragma once
#include <stddef.h>
#include <vector>
#include "utils/Resource.hpp"

/*
non-interleaved layout
*/
class VertexBufferLayout {
public:
    struct Element {
        unsigned count;
        unsigned type;
        size_t offset;
    };

private:
    std::vector<Element> m_elements;
public:
    size_t m_stride;
    VertexBufferLayout() = default;
    VertexBufferLayout(std::vector<Element> const &elements);
    VertexBufferLayout(std::initializer_list<Element> const &elements);
    ~VertexBufferLayout() = default;
    void push(unsigned const count, unsigned type, size_t offset);
    inline unsigned getStride() const { return m_stride; }
    inline std::vector<Element> const &getElements() const { return m_elements; }
};

class InterleavedVertexBufferLayout {
public:
    struct Element {
        unsigned count;
        unsigned type;
    };

private:
    std::vector<Element> m_elements;
    unsigned m_stride = 0;
public:
    InterleavedVertexBufferLayout() = default;
    InterleavedVertexBufferLayout(std::vector<Element> const &elements);
    InterleavedVertexBufferLayout(std::initializer_list<Element> const &elements);
    ~InterleavedVertexBufferLayout() = default;
    void push(unsigned const count, unsigned type);
    inline unsigned getStride() const { return m_stride; }
    inline std::vector<Element> const &getElements() const { return m_elements; }
};

class InstancedArrayLayout {
public:
    struct Element {
        unsigned count;
        unsigned type;
        unsigned divisor;
    };

private:
    std::vector<Element> m_elements;
    unsigned m_stride = 0;
public:
    InstancedArrayLayout() = default;
    InstancedArrayLayout(std::vector<Element> const &elements);
    InstancedArrayLayout(std::initializer_list<Element> const &elements);
    ~InstancedArrayLayout() = default;
    void push(unsigned const count, unsigned type, unsigned divisor);
    inline unsigned getStride() const { return m_stride; }
    inline std::vector<Element> const &getElements() const { return m_elements; }
};

class VertexBuffer : public Resource {
private:
    unsigned m_renderID = 0;
public:
    VertexBuffer(const void *data, size_t size);
    VertexBuffer(size_t size);
    VertexBuffer();
    ~VertexBuffer();
    void bind() const;
    void unbind() const;
};