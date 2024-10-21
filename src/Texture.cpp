#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/gl.h"

#include "Texture.hpp"
#include "GlCall.h"

Texture::Texture(std::string const &filepath) :
    m_RenderID(0), m_FilePath(filepath), m_Buffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {

    stbi_set_flip_vertically_on_load(true);
    m_Buffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    GLCALL(glGenTextures(1, &m_RenderID));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RenderID));

    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));


    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Buffer));
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture() {
    if(m_Buffer) stbi_image_free(m_Buffer); 

    GLCALL(glDeleteTextures(1, &m_RenderID));
}

void Texture::bind(unsigned slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RenderID));
}

void Texture::unbind() const {
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}
