#pragma once
#include "VertexBuffer.hpp"
#include "utils/Resource.hpp"

class VertexArray : public Resource {
private:
    unsigned m_RenderID;
    unsigned m_vertexAttribIndex = 0;
public:
    VertexArray();
    ~VertexArray();
    
    void bind() const;
    void unbind() const;
    void addBuffer(VertexBuffer const &VB, InterleavedVertexBufferLayout const &layout);
    void addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout);
    void addBuffer(VertexBuffer const &VB, InstancedArrayLayout const &layout);
};
