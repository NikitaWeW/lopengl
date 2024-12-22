#pragma once
#include "Texture.hpp"

class Framebuffer {
private:
    unsigned m_rendererID = 0;
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
};