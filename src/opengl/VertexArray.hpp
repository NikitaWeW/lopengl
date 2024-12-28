#pragma once
#include "VertexBuffer.hpp"
#include "utils/Resource.hpp"

class VertexArray : public Resource {
private:
    unsigned m_RenderID;
public:
    VertexArray();
    ~VertexArray();
    
    void bind() const;
    void unbind() const;
    void addBuffer(VertexBuffer const &VB, VertexBufferLayout const &layout);
};
