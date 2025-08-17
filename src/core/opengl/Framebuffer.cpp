#include "Framebuffer.hpp"

ogl::Framebuffer::Framebuffer(unsigned)
{
    glCreateFramebuffers(1, &m_renderID);
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
    return glCheckNamedFramebufferStatus(m_renderID, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void ogl::Framebuffer::attach(Texture const &texture, GLenum attachment)
{
    glNamedFramebufferTexture(getRenderID(), attachment, texture.getRenderID(), 0);
}
void ogl::Framebuffer::attach(TextureMS const &texture, GLenum attachment)
{
    glNamedFramebufferTexture(getRenderID(), attachment, texture.getRenderID(), 0);
}
void ogl::Framebuffer::attach(Cubemap const &cubemap, GLenum attachment)
{
    glNamedFramebufferTexture(getRenderID(), attachment, cubemap.getRenderID(), 0);
}
void ogl::Framebuffer::attach(Renderbuffer const &renderbuffer, GLenum attachment)
{
    glNamedFramebufferRenderbuffer(getRenderID(), attachment, GL_RENDERBUFFER, renderbuffer.getRenderID());
}

ogl::Renderbuffer::Renderbuffer(unsigned)
{
    glCreateRenderbuffers(1, &m_renderID);
}
ogl::Renderbuffer::~Renderbuffer()
{
    if(canDeallocate()) {
        glDeleteRenderbuffers(1, &m_renderID);
    }
}

void ogl::Renderbuffer::bind(unsigned slot) const noexcept { glBindRenderbuffer(GL_RENDERBUFFER, m_renderID); }

