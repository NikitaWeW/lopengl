#pragma once
#include "VertexBuffer.hpp"

class VertexArray {
private:
    unsigned m_RenderID;
public:
    VertexArray();
    ~VertexArray();
    void bind() const;
    void unbind() const;
    void addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout);
};
