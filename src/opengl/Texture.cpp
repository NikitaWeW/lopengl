#include "stb_image.h"
#include "glad/gl.h"
#include <stdexcept>

#include "Texture.hpp"

Texture::Texture(GLenum wrap)
{
    glGenTextures(1, &m_RenderID);
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}

Texture::Texture(GLsizei width, GLsizei height, GLenum wrap) : m_Width(width), m_Height(height)
{
    glGenTextures(1, &m_RenderID);
    glBindTexture(GL_TEXTURE_2D, m_RenderID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

Texture::Texture(std::string const &filepath, bool flip, bool srgb, GLenum wrap) : m_FilePath(filepath)
{
    stbi_set_flip_vertically_on_load(flip);
    m_Buffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    if(!m_Buffer) throw std::runtime_error("failed to load \"" + filepath + "\" texture!"); // TODO: move to m_log

    glGenTextures(1, &m_RenderID);
    glBindTexture(GL_TEXTURE_2D, m_RenderID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(m_Buffer);
    m_Buffer = 0;
}

Texture::~Texture() {
    if(canDeallocate()) {
        glDeleteTextures(1, &m_RenderID);
    }
    m_RenderID = 0;
}

void Texture::bind(unsigned slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RenderID);
}

void Texture::unbind(unsigned slot) const {
    unbindStatic(slot);
}

void Texture::unbindStatic(unsigned slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}


MultisampleTexture::MultisampleTexture(GLsizei width, GLsizei height, GLenum wrap)
{
    glGenTextures(1, &m_RenderID);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_RenderID);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
}

MultisampleTexture::~MultisampleTexture()
{
    if(canDeallocate()) {
        glDeleteTextures(1, &m_RenderID);
    }
}

void MultisampleTexture::bind(unsigned slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void MultisampleTexture::unbind(unsigned slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_RenderID);
}
