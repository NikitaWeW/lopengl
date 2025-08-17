#pragma once
#include "Object.hpp"
#include <cstddef>
#include "glad/gl.h"

namespace ogl
{
    class IndexBuffer : public Object 
    {
    public:
        IndexBuffer() = default;
        IndexBuffer(size_t size, GLenum usage = GL_DYNAMIC_DRAW) noexcept;
        IndexBuffer(size_t size, void const *data, GLenum usage = GL_DYNAMIC_DRAW) noexcept;
        ~IndexBuffer();
        void bind(unsigned slot = 0) const noexcept override;
    }; 
} // namespace ogl
