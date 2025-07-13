#include "Texture.hpp"
#include "stb_image.h"
#include "Bitmap.hpp"
#include <stdexcept>
#include <array>
#include <random>

constexpr unsigned NUM_FACES_IN_CUBEMAP = 6;

ogl::Texture::Texture(GLenum filtermin, GLenum filtermag, GLenum wrap) noexcept
{
    glGenTextures(1, &m_renderID);
    bind();
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtermin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtermag);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
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
    glGenTextures(1, &m_renderID);
    bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}
ogl::TextureMS::~TextureMS()
{
    if(canDeallocate()) {
        glDeleteTextures(1, &m_renderID);
    }
}
void ogl::TextureMS::bind(unsigned slot) const noexcept { glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_renderID); }

glm::vec3 faceCoordsToXYZ(unsigned x, unsigned y, unsigned faceID, unsigned faceSize) 
{
    float A = 2.0f * (float) x / faceSize;
    float B = 2.0f * (float) y / faceSize;

    glm::vec3 res;

    switch (faceID) {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        res = glm::vec3(A - 1.0f, 1.0f, 1.0f - B);
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        res = glm::vec3(1.0f - A, -1.0f, 1.0f - B);
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        res = glm::vec3(1.0f - B, A - 1.0f, 1.0f);
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        res = glm::vec3(B - 1.0f, A - 1.0f, -1.0f);
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        res = glm::vec3(-1.0f, A - 1.0f, 1.0f - B);
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        res = glm::vec3(1.0f, 1.0f - A, 1.0f - B);
        break;
     
    default:
        assert(0);
    }

    return res;
}
// thanks to https://github.com/emeiri/ogldev/blob/master/Common/cubemap_texture.cpp
void convertEquirectangularToCubemap(Bitmap<float> const &equir, std::array<Bitmap<float>, NUM_FACES_IN_CUBEMAP> &cubemapBitmaps)
{
    unsigned faceSize = glm::ceil(equir.getWidth() / 4.0f);

    for (unsigned i = 0; i < NUM_FACES_IN_CUBEMAP; i++) {
        cubemapBitmaps[i] = Bitmap{faceSize, faceSize, equir.getNumComponents()};
    }

    int maxW = equir.getWidth() - 1;
    int maxH = equir.getHeight() - 1;

    for (unsigned face = 0; face < NUM_FACES_IN_CUBEMAP; face++) {
        for (unsigned y = 0; y < faceSize; y++) {
            for (unsigned x = 0; x < faceSize; x++) {
                glm::vec3 P = faceCoordsToXYZ(x, y, face + GL_TEXTURE_CUBE_MAP_POSITIVE_X, faceSize);
                float R = sqrtf(P.x * P.x + P.y * P.y);
                float phi = atan2f(P.y, P.x);
                float theta = atan2f(P.z, R);

                // Calculate texture coordinates
                float u = (float)((phi + M_PI) / (2.0f * M_PI));
                float v = (float((M_PI / 2.0f - theta) / M_PI));

                // Scale texture coordinates by image size
                float U = u * equir.getWidth();
                float V = v * equir.getHeight();

                // 4-samples for bilinear interpolation
                int U1 = glm::clamp<int>(int(floor(U)), 0, maxW);
                int V1 = glm::clamp<int>(int(floor(V)), 0, maxH);
                int U2 = glm::clamp<int>(U1 + 1, 0, maxW);
                int V2 = glm::clamp<int>(V1 + 1, 0, maxH);

                // Calculate the fractional part
                float s = U - U1;
                float t = V - V1;

                // Fetch 4-samples
                glm::vec4 BottomLeft  = equir.getPixel(U1, V1);
                glm::vec4 BottomRight = equir.getPixel(U2, V1);
                glm::vec4 TopLeft     = equir.getPixel(U1, V2);
                glm::vec4 TopRight    = equir.getPixel(U2, V2);

                // Bilinear interpolation
                glm::vec4 color = BottomLeft * (1 - s) * (1 - t) + 
                                  BottomRight * (s) * (1 - t) + 
                                  TopLeft * (1 - s) * t + 
                                  TopRight * (s) * (t);

                cubemapBitmaps[face].setPixel(x, y, color);
            }   // j loop
        }   // i loop
    }   // Face loop
}
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
    std::array<Bitmap<float>, NUM_FACES_IN_CUBEMAP> cubemapBitmaps{};
    convertEquirectangularToCubemap(bitmapImage, cubemapBitmaps);

    // a bit of DSA
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_renderID);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_renderID, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(m_renderID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_renderID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(m_renderID, 1, GL_RGB32F, cubemapBitmaps[0].getWidth(), cubemapBitmaps[0].getHeight());

    for (unsigned i = 0; i < NUM_FACES_IN_CUBEMAP; ++i) {
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
    glGenTextures(1, &m_renderID);
    bind();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
void ogl::Cubemap::bind(unsigned slot) const noexcept 
{ 
    glActiveTexture(GL_TEXTURE0 + slot); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderID); 
}
