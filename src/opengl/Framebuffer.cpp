#include "Framebuffer.hpp"
#include "glad/gl.h"
#include <utility>

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &m_renderID);
}

Framebuffer::~Framebuffer()
{
    if(canDeallocate()) {
        glDeleteFramebuffers(1, &m_renderID);
    }
}

void Framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_renderID); }
void Framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

bool Framebuffer::isComplete()
{
    bind();
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::attach(Texture const &texture, GLenum attachment) 
{
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture.getRenderID(), 0);
}
void Framebuffer::attach(MultisampleTexture const &texture, GLenum attachment)
{
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, texture.getRenderID(), 0);
}

void Framebuffer::attach(Renderbuffer const &renderbuffer, GLenum attachment)
{
    // i hate it
    bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer.getRenderID());
}

void Framebuffer::attach(MultisampleRenderbuffer const &renderbuffer, GLenum attachment)
{
    bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer.getRenderID());
}

Renderbuffer::Renderbuffer()
{
    glGenRenderbuffers(1, &m_renderID);
}

Renderbuffer::Renderbuffer(unsigned format, int width, int height)
{
    glGenRenderbuffers(1, &m_renderID);
    bind();
    glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
}

Renderbuffer::~Renderbuffer()
{
    if(canDeallocate()) {
        glDeleteRenderbuffers(1, &m_renderID);
    }
}

void Renderbuffer::bind()   { glBindRenderbuffer(GL_RENDERBUFFER, m_renderID); }
void Renderbuffer::unbind() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

MultisampleRenderbuffer::MultisampleRenderbuffer()
{
    glGenRenderbuffers(1, &m_renderID);
}

MultisampleRenderbuffer::MultisampleRenderbuffer(unsigned format, int width, int height)
{
    glGenRenderbuffers(1, &m_renderID);
    bind();
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, format, width, height);
}

MultisampleRenderbuffer::~MultisampleRenderbuffer()
{
    if(canDeallocate()) {
        glDeleteRenderbuffers(1, &m_renderID);
    }
}

void MultisampleRenderbuffer::bind()   { glBindRenderbuffer(GL_RENDERBUFFER, m_renderID); }
void MultisampleRenderbuffer::unbind() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }