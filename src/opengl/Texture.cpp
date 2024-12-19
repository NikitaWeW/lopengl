#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/gl.h"
#include <stdexcept>

#include "Texture.hpp"

Texture::Texture(std::string const &filepath, bool flip) :
    m_FilePath(filepath) {

    stbi_set_flip_vertically_on_load(flip);
    m_Buffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    if(!m_Buffer) throw std::runtime_error("failed to load \"" + filepath + "\" texture!"); // TODO: move to m_log

    glGenTextures(1, &m_RenderID);
    glBindTexture(GL_TEXTURE_2D, m_RenderID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(m_Buffer);
    m_managing = true;
}

Texture::Texture(Texture const &other) {
    copy(other);
}
Texture::Texture(Texture &&other) {
    swap(std::forward<Texture>(other));
}
void Texture::operator=(Texture const &other) {
    copy(other);
}
void Texture::copy(Texture const &other) {
    std::swap(m_managing, other.m_managing);
    m_FilePath = other.m_FilePath;
    m_Buffer = other.m_Buffer;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_BPP = other.m_BPP;
    m_RenderID = other.m_RenderID;
    type = other.type;
}
void Texture::swap(Texture &&other) {
    std::swap(m_managing, other.m_managing);
    std::swap(m_RenderID, other.m_RenderID);
    std::swap(m_FilePath, other.m_FilePath);
    std::swap(m_Buffer, other.m_Buffer);
    std::swap(m_Width, other.m_Width);
    std::swap(m_Height, other.m_Height);
    std::swap(m_BPP, other.m_BPP);
    std::swap(type, other.type);
}
Texture::~Texture() {
    if(m_managing) {
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
