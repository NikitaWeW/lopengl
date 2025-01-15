#pragma once
#include "Texture.hpp"
#include "utils/Resource.hpp"

class Renderbuffer : public Resource {
private:
    unsigned m_renderID = 0;
public:
    Renderbuffer();
    Renderbuffer(unsigned format, int width, int height);
    ~Renderbuffer();

    void bind();
    void unbind();

    inline unsigned getRenderID() const { return m_renderID; }
};

class MultisampleRenderbuffer : public Resource {
private:
    unsigned m_renderID = 0;
public:
    MultisampleRenderbuffer();
    MultisampleRenderbuffer(unsigned format, int width, int height);
    ~MultisampleRenderbuffer();

    void bind();
    void unbind();

    inline unsigned getRenderID() const { return m_renderID; }
};

class Framebuffer : Resource {
private:
    unsigned m_renderID = 0;
public:
    Framebuffer();
    ~Framebuffer();

    void bind();
    void unbind();

    bool isComplete();

    void attach(Texture const &texture, GLenum attachment = GL_COLOR_ATTACHMENT0);
    void attach(MultisampleTexture const &texture, GLenum attachment = GL_COLOR_ATTACHMENT0);
    void attach(Renderbuffer const &renderbuffer, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT);
    void attach(MultisampleRenderbuffer const &renderbuffer, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT);

    inline unsigned getRenderID() const { return m_renderID; }
};
