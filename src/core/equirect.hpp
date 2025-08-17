/*
                      _               _   
      ___  __ _ _   _(_)_ __ ___  ___| |_ 
     / _ \/ _` | | | | | '__/ _ \/ __| __|    Copyright (c) 2025 Nikita Martynau
    |  __/ (_| | |_| | | | |  __/ (__| |_     https://opensource.org/license/mit
     \___|\__, |\__,_|_|_|  \___|\___|\__|    <todo: insert repo name here>
             |_|                          

    Thanks to https://github.com/emeiri/ogldev/blob/master/Common/cubemap_texture.cpp
    A small library for converting equirectangular images from cubemaps and vice versa.
*/
/*
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "glm/gtc/constants.hpp"
#include <array>

#define EQUIRECT_ASSERT(x, msg) assert((x) && (msg))

/**
 * \brief A bitmap class, representing a 2d texture with the n number of components.
 * \tparam Format_t The underlying type.
 * \tparam components The number of glm::vec components used in the setPixel and getPixel operations.
 */
template <typename Format_t = float, size_t components = 4>
class Bitmap
{
private:
    unsigned m_width, m_height, m_numComponents;
    std::vector<Format_t> m_data;

    inline size_t getOffsetOf(unsigned x, unsigned y) const { return m_numComponents * (y * m_width + x); }
public:
    Bitmap() = default;
    Bitmap(unsigned width, unsigned height, unsigned numComponents, Format_t const *data = nullptr);

    /**
     * \brief Sets the pixel at the (x; y) absolute coordinates to a specified value.
     * \param x The x coordinate in range of [0; width).
     * \param x The y coordinate in range of [0; height).
     * \param value The 4-component value. Only first numComponents will be set.
     */
    void setPixel(unsigned x, unsigned y, glm::vec<components, Format_t> const &value);
    /**
     * \brief Gets the pixel at (x, y).
     * \param x The x coordinate in range of [0; width).
     * \param x The y coordinate in range of [0; height).
     * \return The value at the (x; y) absolute coordinates. Only first numComponents will be filled, the rest will be filled with 0.
     */
    glm::vec<components, Format_t> getPixel(unsigned x, unsigned y) const;

    unsigned getWidth() const;
    unsigned getHeight() const;
    unsigned getNumComponents() const;
    glm::vec2 getDimensions() const;
    Format_t const *getData() const;
    Format_t *getData();
};

namespace eqr
{
    /**
     * \brief OpenGL-style right-handed cube map faces indices.
     */
    enum CubemapFaces
    {
        POS_X = 0,
        NEG_X = 1,
        POS_Y = 2,
        NEG_Y = 3,
        POS_Z = 4,
        NEG_Z = 5,
        NUM_CUBEMAP_FACES = 6
    };

    /**
     * \brief Convert a cubemap to an equirectangular image.
     * \param cubemapFaces An array of cubemap face bitmaps in the order of CubemapFaces enum of size CubemapFaces::NUM_CUBEMAP_FACES (6). The size and number of components of each face must be consistent.
     * \tparam T The type of a bitmap. May be deduced.
     * \return A 2x1 bitmap of the size x = 4*cubemap_face_size; y = 2*cubemap_face_size with the same number of components as in the cubemap face, representing the equirectangular image.
     */
    template <typename T>
    Bitmap<T> fromCubemap(std::array<Bitmap<T>, NUM_CUBEMAP_FACES> const &cubemapFaces);

    /**
     * \brief Convert an equirectangular image to a cubemap.
     * \param equirectangularImage A 2x1 bitmap of the size x = 4*cubemap_face_size; y = 2*cubemap_face_size with the same number of components as in the cubemap face, representing the equirectangular image.
     * \tparam T The type of a bitmap. May be deduced.
     * \return An array of square cubemap face bitmaps in the order of CubemapFaces enum of size CubemapFaces::NUM_CUBEMAP_FACES (6). The size and number of components of each face is consistent. The size is equal to equirectangular_width / 4 = equirectangular_height / 2.
     */
    template <typename T>
    std::array<Bitmap<T>, NUM_CUBEMAP_FACES> toCubemap(Bitmap<T> const &equirectangularImage);
} // namespace eqr


// ===============
// Implementation 
// ===============

namespace eqr
{
    struct uvFace
    {
        int face;
        glm::vec2 uv;
    };

    inline uvFace getUVface(glm::vec3 dir)
    { // dunno why i had to invert some directions, probably because of that coordinate system from faceCoordsToXYZ.
        dir = glm::normalize(dir);
        glm::vec3 adir = glm::abs(dir);

        if (adir.x >= adir.y && adir.x >= adir.z)
        {
            // +X or -X
            if (dir.x > 0.0f)
            {
                return uvFace {
                    NEG_Z,
                    {
                        (-dir.z / adir.x + 1.0f) * 0.5f,
                        (-dir.y / adir.x + 1.0f) * 0.5f
                    }
                };
            }
            else
            {
                return uvFace {
                    POS_Z,
                    {
                        ( dir.z / adir.x + 1.0f) * 0.5f,
                        (-dir.y / adir.x + 1.0f) * 0.5f
                    }
                };
            }
        }
        else if (adir.y >= adir.x && adir.y >= adir.z)
        {
            // +Y or -Y
            if (dir.y > 0.0f)
            {
                return uvFace {
                    POS_Y,
                    {
                        ( dir.z / adir.y + 1.0f) * 0.5f, 
                        1.0f - ( dir.x / adir.y + 1.0f) * 0.5f, 
                    }
                };
            }
            else
            {
                return uvFace {
                    NEG_Y,
                    {
                        ( dir.z / adir.y + 1.0f) * 0.5f, 
                        ( dir.x / adir.y + 1.0f) * 0.5f, 
                    }
                };
            }
        }
        else
        {
            // +Z or -Z
            if (dir.z > 0.0f)
            {
                return uvFace {
                    POS_X,
                    {
                        ( dir.x / adir.z + 1.0f) * 0.5f,
                        (-dir.y / adir.z + 1.0f) * 0.5f
                    }
                };
            }
            else
            {
                return uvFace {
                    NEG_X,
                    {
                        (-dir.x / adir.z + 1.0f) * 0.5f,
                        (-dir.y / adir.z + 1.0f) * 0.5f
                    }
                };
            }
        }
    }

    template <typename T>
    inline Bitmap<T> fromCubemap(std::array<Bitmap<T>, NUM_CUBEMAP_FACES> const &cubemapFaces)
    {
        // assume all faces are square and same size
        unsigned faceSize = cubemapFaces[0].getWidth();
        unsigned width = faceSize * 4;
        unsigned height = faceSize * 2;
        unsigned numComponents = cubemapFaces[0].getNumComponents();
        unsigned maxFaceTexel = faceSize - 1;

        Bitmap<T> equirectangularImage{width, height, numComponents};
        for(unsigned y = 0; y < height; ++y)
        {
            float normalized_y = ((float) y + 0.5f) / height;

            // polar
            float theta = glm::half_pi<float>() - normalized_y * glm::pi<float>();
            for(unsigned x = 0; x < width; ++x)
            {
                float normalized_x = ((float) x + 0.5f) / width;

                // azimuth
                float phi = normalized_x * glm::two_pi<float>() - glm::pi<float>();


                // direction on unit sphere
                glm::vec3 dir(
                    glm::cos(theta) * glm::cos(phi),
                    glm::sin(theta),
                    glm::cos(theta) * glm::sin(phi)
                );

                auto [face, uv] = getUVface(dir);

                EQUIRECT_ASSERT(uv.x >= 0 && uv.y >= 0 && uv.x <= 1 && uv.y <= 1, "uv is not in range of [0; 1]");

                glm::vec2 texel = uv * static_cast<float>(maxFaceTexel);

                // 4-samples for bilinear interpolation
                int U1 = glm::clamp<int>(int(glm::floor(texel.x)), 0, maxFaceTexel);
                int V1 = glm::clamp<int>(int(glm::floor(texel.y)), 0, maxFaceTexel);
                int U2 = glm::clamp<int>(U1 + 1, 0, maxFaceTexel);
                int V2 = glm::clamp<int>(V1 + 1, 0, maxFaceTexel);

                // Calculate the fractional part
                float s = texel.x - U1;
                float t = texel.y - V1;

                // Fetch 4-samples
                glm::vec4 BottomLeft  = cubemapFaces[face].getPixel(U1, V1);
                glm::vec4 BottomRight = cubemapFaces[face].getPixel(U2, V1);
                glm::vec4 TopLeft     = cubemapFaces[face].getPixel(U1, V2);
                glm::vec4 TopRight    = cubemapFaces[face].getPixel(U2, V2);

                // Bilinear interpolation
                glm::vec4 color = 
                    BottomLeft * (1 - s) * (1 - t) + 
                    BottomRight * (s) * (1 - t) + 
                    TopLeft * (1 - s) * t + 
                    TopRight * (s) * (t);

                equirectangularImage.setPixel(x, y, color);

            } // for x
        } // for y

        return equirectangularImage;
    } // fromCubemap

    inline glm::vec3 faceCoordsToXYZ(unsigned x, unsigned y, unsigned face, unsigned faceSize) 
    {
        float A = 2.0f * (float) x / faceSize;
        float B = 2.0f * (float) y / faceSize;

        glm::vec3 res;

        switch (face) {
        case POS_X:
            res = glm::vec3(A - 1.0f, 1.0f, 1.0f - B);
            break;
        case NEG_X:
            res = glm::vec3(1.0f - A, -1.0f, 1.0f - B);
            break;
        case POS_Y:
            res = glm::vec3(1.0f - B, A - 1.0f, 1.0f);
            break;
        case NEG_Y:
            res = glm::vec3(B - 1.0f, A - 1.0f, -1.0f);
            break;
        case POS_Z:
            res = glm::vec3(-1.0f, A - 1.0f, 1.0f - B);
            break;
        case NEG_Z:
            res = glm::vec3(1.0f, 1.0f - A, 1.0f - B);
            break;
        
        default:
            EQUIRECT_ASSERT(false, "unknown face");
        }

        return res;
    }
    template <typename T>
    inline std::array<Bitmap<T>, NUM_CUBEMAP_FACES> toCubemap(Bitmap<T> const &equirectangularImage)
    {
        std::array<Bitmap<T>, NUM_CUBEMAP_FACES> cubemapBitmaps;
        // unsigned faceSize = glm::ceil(equirectangularImage.getWidth() / 4.0f);
        unsigned faceSize = glm::ceil(equirectangularImage.getHeight() / 2.0f);

        for (unsigned i = 0; i < NUM_CUBEMAP_FACES; i++) {
            cubemapBitmaps[i] = Bitmap{faceSize, faceSize, equirectangularImage.getNumComponents()};
        }

        int maxW = equirectangularImage.getWidth() - 1;
        int maxH = equirectangularImage.getHeight() - 1;

        for (unsigned face = 0; face < NUM_CUBEMAP_FACES; face++) {
            for (unsigned y = 0; y < faceSize; y++) {
                for (unsigned x = 0; x < faceSize; x++) {
                    glm::vec3 P = faceCoordsToXYZ(x, y, face, faceSize);
                    float R = glm::sqrt(P.x * P.x + P.y * P.y);
                    float phi = atan2f(P.y, P.x);
                    float theta = atan2f(P.z, R);

                    // Calculate texture coordinates
                    float u = (phi + glm::pi<float>()) / (glm::two_pi<float>());
                    float v = (glm::half_pi<float>() - theta) / glm::pi<float>();

                    // Scale texture coordinates by image size
                    float U = u * equirectangularImage.getWidth();
                    float V = v * equirectangularImage.getHeight();

                    // 4-samples for bilinear interpolation
                    int U1 = glm::clamp<int>(int(glm::floor(U) + 0.5), 0, maxW);
                    int V1 = glm::clamp<int>(int(glm::floor(V) + 0.5), 0, maxH);
                    int U2 = glm::clamp<int>(U1 + 1, 0, maxW);
                    int V2 = glm::clamp<int>(V1 + 1, 0, maxH);

                    // Calculate the fractional part
                    float s = U - U1;
                    float t = V - V1;

                    // Fetch 4-samples
                    glm::vec4 BottomLeft  = equirectangularImage.getPixel(U1, V1);
                    glm::vec4 BottomRight = equirectangularImage.getPixel(U2, V1);
                    glm::vec4 TopLeft     = equirectangularImage.getPixel(U1, V2);
                    glm::vec4 TopRight    = equirectangularImage.getPixel(U2, V2);

                    // Bilinear interpolation
                    glm::vec4 color = 
                        BottomLeft  * (1 - s) * (1 - t) + 
                        BottomRight * (s)     * (1 - t) + 
                        TopLeft     * (1 - s) * (t) + 
                        TopRight    * (s)     * (t);

                    cubemapBitmaps[face].setPixel(x, y, color);
                }   // j loop
            }   // i loop
        }   // Face loop

        return cubemapBitmaps;
    } // toCubemap
} // namespace eqr


template <typename Format_t, size_t components>
inline Bitmap<Format_t, components>::Bitmap(unsigned width, unsigned height, unsigned numComponents, Format_t const *src) : m_width(width), m_height(height), m_numComponents(numComponents)
{
    EQUIRECT_ASSERT(m_numComponents <= 4, "Components > 4 is yet not supported!");
    m_data.resize(width * height * numComponents);
    if(src) {
        std::copy(src, src + m_data.size(), m_data.begin());
    }
}

template <typename Format_t, size_t components>
inline void Bitmap<Format_t, components>::setPixel(unsigned x, unsigned y, glm::vec<components, Format_t> const &value)
{
    EQUIRECT_ASSERT(x < m_width && y < m_height, "x or y is out of range!");
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    Format_t *data = m_data.data();
    size_t offset = getOffsetOf(x, y);
    if (m_numComponents > 0) data[offset + 0] = value.x;
    if (m_numComponents > 1) data[offset + 1] = value.y;
    if (m_numComponents > 2) data[offset + 2] = value.z;
    if (m_numComponents > 3) data[offset + 3] = value.w;
}

template <typename Format_t, size_t components>
inline glm::vec<components, Format_t> Bitmap<Format_t, components>::getPixel(unsigned x, unsigned y) const
{
    EQUIRECT_ASSERT(x < m_width && y < m_height, "x or y is out of range!");
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    Format_t const *data = m_data.data();
    size_t offset = getOffsetOf(x, y);
    return glm::vec4(
        m_numComponents > 0 ? data[offset + 0] : 0.0f,
        m_numComponents > 1 ? data[offset + 1] : 0.0f,
        m_numComponents > 2 ? data[offset + 2] : 0.0f,
        m_numComponents > 3 ? data[offset + 3] : 0.0f
    );
}

template <typename Format_t, size_t components> 
inline unsigned Bitmap<Format_t, components>::getWidth() const 
{ 
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    return m_width; 
}
template <typename Format_t, size_t components> 
inline unsigned Bitmap<Format_t, components>::getHeight() const 
{ 
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    return m_height; 
}
template <typename Format_t, size_t components> 
inline unsigned Bitmap<Format_t, components>::getNumComponents() const 
{ 
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    return m_numComponents; 
}
template <typename Format_t, size_t components> 
inline glm::vec2 Bitmap<Format_t, components>::getDimensions() const 
{ 
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    return glm::vec2{getWidth(), getHeight()}; 
}
template <typename Format_t, size_t components> 
inline Format_t const *Bitmap<Format_t, components>::getData() const 
{ 
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    return m_data.data(); 
}
template <typename Format_t, size_t components> 
inline Format_t *Bitmap<Format_t, components>::getData() 
{ 
    EQUIRECT_ASSERT(m_data.size() == m_height * m_width * m_numComponents, "Bitmap not initialized!");
    return m_data.data(); 
}
