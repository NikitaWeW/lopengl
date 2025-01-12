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

    void attachTexture(Texture const &texture);
    void attachTexture(MultisampleTexture const &texture);
    void attachRenderbuffer(Renderbuffer const &renderbuffer);
    void attachRenderbuffer(MultisampleRenderbuffer const &renderbuffer);

    inline unsigned getRenderID() const { return m_renderID; }
};
