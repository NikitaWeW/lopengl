#pragma once
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"

size_t getSizeOfGLType(unsigned type);

class Renderer {
private:

public:
    void Clear(float r = 0, float g = 0, float b = 0) const;
    void Draw(VertexArray const &va, IndexBuffer const &ib, Shader const &shader) const;
};