#pragma once
#include "VertexBuffer.hpp"
#include "utils/Resource.hpp"

class VertexArray : public Resource {
private:
    unsigned m_RenderID;
    unsigned m_vertexAttribIndex = 0;
public:
    template<typename Layout> VertexArray(VertexBuffer const &VB, Layout const &layout);
    VertexArray();
    ~VertexArray();
    
    void bind() const;
    void unbind() const;
    void addBuffer(VertexBuffer const &VB, InterleavedVertexBufferLayout const &layout);
    void addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout);
    void addBuffer(VertexBuffer const &VB, InstancedArrayLayout const &layout);
};

template <typename Layout> inline VertexArray::VertexArray(VertexBuffer const &VB, Layout const &layout) { 
    glGenVertexArrays(1, &m_RenderID);
    addBuffer(VB, layout); 
}
