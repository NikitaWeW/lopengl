#include "Framebuffer.hpp"
#include "glad/gl.h"
#include <utility>

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &m_renderID);
    m_managing = true;
}

Framebuffer::~Framebuffer()
{
    if(m_managing) {
        glDeleteFramebuffers(1, &m_renderID);
    }
}

Framebuffer::Framebuffer(Framebuffer const &other) { copy(other); }
Framebuffer::Framebuffer(Framebuffer &&other) { swap(std::forward<Framebuffer &&>(other)); }
void Framebuffer::operator=(Framebuffer const &other) { copy(other); }

void Framebuffer::copy(Framebuffer const &other)
{
    std::swap(m_managing, other.m_managing);
    m_renderID = other.m_renderID;
}

void Framebuffer::swap(Framebuffer &&other)
{
    std::swap(m_managing, other.m_managing);
    std::swap(m_renderID, other.m_renderID);
}

void Framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_renderID); }
void Framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

bool Framebuffer::isComplete()
{
    bind();
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::attachTexture(Texture const &texture)
{
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getRenderID(), 0);
}

void Framebuffer::attachRenderbuffer(Renderbuffer const &renderbuffer)
{
    // i hate it
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer.getRenderID());
}

Renderbuffer::Renderbuffer()
{
    glGenRenderbuffers(1, &m_renderID);
    m_managing = true;
}

Renderbuffer::Renderbuffer(unsigned format, unsigned width, unsigned height) : m_format(format), m_width(width), m_height(height)
{
}

Renderbuffer::~Renderbuffer()
{
    if(m_managing) {
        glDeleteRenderbuffers(1, &m_renderID);
    }
}

Renderbuffer::Renderbuffer(Renderbuffer const &other) { copy(other); }
Renderbuffer::Renderbuffer(Renderbuffer &&other) { swap(std::forward<Renderbuffer &&>(other)); }
void Renderbuffer::operator=(Renderbuffer const &other) { copy(other); }

void Renderbuffer::copy(Renderbuffer const &other)
{
    std::swap(m_managing, other.m_managing);
    m_renderID = other.m_renderID;
}

void Renderbuffer::swap(Renderbuffer &&other)
{
    std::swap(m_managing, other.m_managing);
    std::swap(m_renderID, other.m_renderID);
}

void Renderbuffer::bind() { glBindRenderbuffer(GL_RENDERBUFFER, m_renderID); }
void Renderbuffer::unbind() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }
