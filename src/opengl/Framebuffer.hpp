#pragma once
#include "Texture.hpp"
// TODO: make Resource class responsible for m_managing field
// TODO: refactor renderbuffer if needed
class Renderbuffer {
private:
    unsigned m_renderID = 0;
    mutable bool m_managing = false;
    unsigned m_format, m_width, m_height;
public:
    Renderbuffer();
    Renderbuffer(unsigned format, unsigned width, unsigned height);
    ~Renderbuffer();
    Renderbuffer(Renderbuffer const &other);
    Renderbuffer(Renderbuffer &&other);
    void operator=(Renderbuffer const &other);
    void copy(Renderbuffer const &other);
    void swap(Renderbuffer &&other);

    void bind();
    void unbind();

    inline unsigned getFormat()   const { return m_format; }
    inline unsigned getWidth()    const { return m_width; }
    inline unsigned getHeight()   const { return m_height; }
    inline unsigned getRenderID() const { return m_renderID; }
};

class Framebuffer {
private:
    unsigned m_renderID = 0;
    mutable bool m_managing = false;
public:
    Framebuffer();
    ~Framebuffer();
    Framebuffer(Framebuffer const &other);
    Framebuffer(Framebuffer &&other);
    void operator=(Framebuffer const &other);
    void copy(Framebuffer const &other);
    void swap(Framebuffer &&other);

    void bind();
    void unbind();

    bool isComplete();

    void attachTexture(Texture const &texture);
    void attachRenderbuffer(Renderbuffer const &renderbuffer);
};
