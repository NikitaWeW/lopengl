#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/gl.h"

#include "Texture.hpp"

Texture::Texture(std::string const &filepath) :
    m_RenderID(0), m_FilePath(filepath), m_Buffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {

    stbi_set_flip_vertically_on_load(true);
    m_Buffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    glGenTextures(1, &m_RenderID);
    glBindTexture(GL_TEXTURE_2D, m_RenderID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if(m_Buffer) stbi_image_free(m_Buffer); 

    glDeleteTextures(1, &m_RenderID);
}

void Texture::bind(unsigned slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RenderID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
