#pragma once
#include "VertexBuffer.hpp"

class VertexArray {
private:
    unsigned m_RenderID;
    mutable bool m_managing = true;
public:
    VertexArray();
    ~VertexArray();
    VertexArray(VertexArray const &other);
    VertexArray(VertexArray &&other);
    void operator=(VertexArray const &other);
    void copy(VertexArray const &other);
    void swap(VertexArray &&other);
    void bind() const;
    void unbind() const;
    void addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout);
};
