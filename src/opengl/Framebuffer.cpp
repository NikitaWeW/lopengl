#include "Framebuffer.hpp"

ogl::Framebuffer::Framebuffer(unsigned)
{
    glGenFramebuffers(1, &m_renderID);
}
ogl::Framebuffer::~Framebuffer()
{
    if(canDeallocate()) {
        glDeleteFramebuffers(1, &m_renderID);
    }
}

void ogl::Framebuffer::bind(unsigned slot) const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
}
bool ogl::Framebuffer::isComplete()
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void ogl::Framebuffer::attach(Texture const &texture, GLenum attachment)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture.getRenderID(), 0);
}
void ogl::Framebuffer::attach(TextureMS const &texture, GLenum attachment)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, texture.getRenderID(), 0);
}
void ogl::Framebuffer::attach(Cubemap const &cubemap, GLenum attachment)
{
    glFramebufferTexture(GL_FRAMEBUFFER, attachment, cubemap.getRenderID(), 0);
}
void ogl::Framebuffer::attach(Renderbuffer const &renderbuffer, GLenum attachment)
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer.getRenderID());
}
void ogl::Framebuffer::attach(RenderbufferMS const &renderbuffer, GLenum attachment)
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer.getRenderID());
}

ogl::Renderbuffer::Renderbuffer(unsigned)
{
    glGenRenderbuffers(1, &m_renderID);
}
ogl::Renderbuffer::~Renderbuffer()
{
    if(canDeallocate()) {
        glDeleteRenderbuffers(1, &m_renderID);
    }
}

void ogl::Renderbuffer::bind(unsigned slot) const noexcept { glBindRenderbuffer(GL_RENDERBUFFER, m_renderID); }

ogl::RenderbufferMS::RenderbufferMS(unsigned)
{
    glGenRenderbuffers(1, &m_renderID);
}
ogl::RenderbufferMS::~RenderbufferMS()
{
    if(canDeallocate()) {
        glDeleteRenderbuffers(1, &m_renderID);
    }
}

void ogl::RenderbufferMS::bind(unsigned slot) const noexcept { glBindRenderbuffer(GL_RENDERBUFFER, m_renderID); }
