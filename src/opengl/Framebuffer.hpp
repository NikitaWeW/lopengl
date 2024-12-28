#pragma once
#include "Texture.hpp"
#include "utils/Resource.hpp"

// TODO: refactor renderbuffer if needed
class Renderbuffer : public Resource {
private:
    unsigned m_renderID = 0;
    unsigned m_format, m_width, m_height;
public:
    Renderbuffer();
    Renderbuffer(unsigned format, unsigned width, unsigned height);
    ~Renderbuffer();

    void bind();
    void unbind();

    inline unsigned getFormat()   const { return m_format; }
    inline unsigned getWidth()    const { return m_width; }
    inline unsigned getHeight()   const { return m_height; }
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

    void attachTexture(Texture const &texture);
    void attachRenderbuffer(Renderbuffer const &renderbuffer);
};
