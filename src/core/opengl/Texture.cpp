#include "Texture.hpp"
#include "stb_image.h"
#include "../equirect.hpp"
#include <stdexcept>
#include <array>
#include <random>

ogl::Texture::Texture(GLenum filtermin, GLenum filtermag, GLenum wrap) noexcept
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_renderID);
    
    glTextureParameteri(m_renderID, GL_TEXTURE_MIN_FILTER, filtermin);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAG_FILTER, filtermag);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_T, wrap);
}
ogl::Texture::Texture(std::filesystem::path const &filepath, bool flip, bool srgb, std::string const &type, bool *isGrayScalePtr) : type(type)
{
    stbi_set_flip_vertically_on_load(flip);
    int width = 0, height = 0, numComponents = 0;
    unsigned char *buffer = nullptr;
    buffer = stbi_load(static_cast<char const *>(filepath.string().c_str()), &width, &height, &numComponents, 4);
    if(!buffer) throw std::runtime_error{"failed to load a texture " + filepath.string()};
    assert(width > 0 && height > 0);

    if(isGrayScalePtr) {
        const int pixelCount = glm::min(width * height, 10);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, (width - 1) * (height - 1));
        *isGrayScalePtr = true;
        for (int i = 0; i < pixelCount; ++i) {
            int index = dist(gen);
            unsigned char r = buffer[index * numComponents + 0];
            unsigned char g = buffer[index * numComponents + 1];
            unsigned char b = buffer[index * numComponents + 2];
            if (r != g || r != b) {
                *isGrayScalePtr = false;
                break;
            }
        }
    }

    glGenTextures(1, &m_renderID);
    bind();
    
    if(width * height > 10000) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(buffer);
}
ogl::Texture::~Texture()
{
    if(canDeallocate()) {
        glDeleteTextures(1, &m_renderID);
    }
}
void ogl::Texture::bind(unsigned slot) const noexcept 
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_renderID);
}

ogl::TextureMS::TextureMS(GLenum filter, GLenum wrap) noexcept
{
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_renderID);
}
ogl::TextureMS::~TextureMS()
{
    if(canDeallocate()) {
        glDeleteTextures(1, &m_renderID);
    }
}
void ogl::TextureMS::bind(unsigned slot) const noexcept { glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_renderID); }

ogl::Cubemap::Cubemap(std::filesystem::path const &filepath, bool flip)
{
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(flip);
    float *image = stbi_loadf(static_cast<char const *>(filepath.string().c_str()), &width, &height, &numChannels, 0);
    if(!image) {
        throw std::runtime_error{"failed to load an image: " + filepath.string()};
    }

    Bitmap<float> const bitmapImage{static_cast<unsigned>(width), static_cast<unsigned>(height), static_cast<unsigned>(numChannels), image};
    stbi_image_free(image);
    std::array<Bitmap<float>, eqr::NUM_CUBEMAP_FACES> cubemapBitmaps = eqr::toCubemap(bitmapImage);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_renderID);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(
        m_renderID, 
        1, 
        GL_RGB16F, 
        cubemapBitmaps[0].getWidth(), 
        cubemapBitmaps[0].getHeight()
    );

    for (unsigned i = 0; i < eqr::NUM_CUBEMAP_FACES; ++i) {
        const void* sourceImage = cubemapBitmaps[i].getData();
        glTextureSubImage3D(
            m_renderID, 
            0,      // mipmap level
            0,      // xOffset
            0,      // yOffset
            i,      // zOffset (layer in the case of a cubemap)
            cubemapBitmaps[0].getWidth(), cubemapBitmaps[0].getHeight(),   // 2D image dimensions
            1,          // depth
            GL_RGB,     // format
            GL_FLOAT,   // data type
            sourceImage
        );
    }
}

ogl::Cubemap::Cubemap(unsigned) noexcept
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_renderID);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void ogl::Cubemap::bind(unsigned slot) const noexcept 
{ 
    glActiveTexture(GL_TEXTURE0 + slot); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderID); 
}
