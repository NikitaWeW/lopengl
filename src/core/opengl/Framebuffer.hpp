#pragma once
#include "Object.hpp"
#include "glad/gl.h"
#include "Texture.hpp"

namespace ogl
{
    class Renderbuffer : public Object {
    public:
        Renderbuffer() = default;
        explicit Renderbuffer(unsigned); // dummy argument, constructor generates object
        ~Renderbuffer();
    
        void bind(unsigned slot = 0) const noexcept override;
    };

    class Framebuffer : public Object {
    public:
        Framebuffer() = default;
        explicit Framebuffer(unsigned); // dummy argument, constructor generates object
        ~Framebuffer();
        void bind(unsigned slot = 0) const noexcept override;
        bool isComplete();
        void attach(Texture const &texture, GLenum attachment = GL_COLOR_ATTACHMENT0);
        void attach(TextureMS const &texture, GLenum attachment = GL_COLOR_ATTACHMENT0);
        void attach(Cubemap const &cubemap, GLenum attachment = GL_COLOR_ATTACHMENT0);
        void attach(Renderbuffer const &renderbuffer, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT);
    };
} // namespace ogl
