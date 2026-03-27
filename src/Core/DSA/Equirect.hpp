#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "glm/gtc/constants.hpp"
#include <array>
#include "Bitmap.hpp"

#define EQUIRECT_ASSERT(x, msg) assert(x && msg)

namespace eqr
{
    /// @brief OpenGL-style right-handed cube map faces indices.
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

    /// @brief Convert a cubemap to an equirectangular image.
    /// @param cubemapFaces An array of cubemap face bitmaps in the order of CubemapFaces enum of size CubemapFaces::NUM_CUBEMAP_FACES (6). The size and number of components of each face must be consistent.
    /// @tparam T The type of a bitmap. May be deduced.
    /// @return A 2x1 bitmap of the size x = 4*cubemap_face_size; y = 2*cubemap_face_size with the same number of components as in the cubemap face, representing the equirectangular image.
    template <typename T>
    Bitmap<T> fromCubemap(std::array<Bitmap<T>, NUM_CUBEMAP_FACES> const &cubemapFaces);

    /// @brief Convert an equirectangular image to a cubemap.
    /// @param equirectangularImage A 2x1 bitmap of the size x = 4*cubemap_face_size; y = 2*cubemap_face_size with the same number of components as in the cubemap face, representing the equirectangular image.
    /// @tparam T The type of a bitmap. May be deduced.
    /// @return An array of square cubemap face bitmaps in the order of CubemapFaces enum of size CubemapFaces::NUM_CUBEMAP_FACES (6). The size and number of components of each face is consistent. The size is equal to equirectangular_width / 4 = equirectangular_height / 2.
    template <typename T>
    std::array<Bitmap<T>, NUM_CUBEMAP_FACES> toCubemap(Bitmap<T> const &equirectangularImage);
} // namespace eqr


// ===============
// Implementation 
// ===============

/// @cond Doxygen_Suppress
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
        unsigned faceSize = glm::ceil(equirectangularImage.getWidth() / 4.0f);

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

/// @endcond
