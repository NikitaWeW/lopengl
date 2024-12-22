#include "Framebuffer.hpp"
#include "glad/gl.h"
#include <utility>

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &m_rendererID);
    m_managing = true;
}

Framebuffer::~Framebuffer()
{
    if(m_managing) {
        glDeleteFramebuffers(1, &m_rendererID);
    }
}

Framebuffer::Framebuffer(Framebuffer const &other) { copy(other); }
Framebuffer::Framebuffer(Framebuffer &&other) { swap(std::forward<Framebuffer &&>(other)); }
void Framebuffer::operator=(Framebuffer const &other) { copy(other); }

void Framebuffer::copy(Framebuffer const &other)
{
    std::swap(m_managing, other.m_managing);
    m_rendererID = other.m_rendererID;
}

void Framebuffer::swap(Framebuffer &&other)
{
    std::swap(m_managing, other.m_managing);
    std::swap(m_rendererID, other.m_rendererID);
}

void Framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID); }
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
