#pragma once
#include "glm/glm.hpp"
#include <vector>
#include "Core/Logging.hpp"

/// @brief A bitmap class, representing a 2d image with the n number of components.
/// @tparam Format_t The underlying type.
/// @tparam components The number of glm::vec components used in the setPixel and getPixel operations. Does not affect the number of components in the image. 
template <typename Format_t = float>
class Bitmap
{
private:
    unsigned m_width, m_height, m_numComponents;
    std::vector<Format_t> m_data;

    inline std::size_t getOffsetOf(unsigned x, unsigned y) const { return m_numComponents * (y * m_width + x); }
public:
    Bitmap() = default;

    /// @brief Construct a valid bitmap.
    /// @param width, height Dimensions of the bitmap
    /// @param numComponents The number of channels / Format_t's per pixel
    /// @param data The optional  If not provided, the bitmap is filled with default values.
    Bitmap(unsigned width, unsigned height, unsigned numComponents, Format_t const *data = nullptr);

    /// @brief Sets the pixel at the (x; y) absolute coordinates to a specified value.
    /// @param x The x coordinate in range of [0; width).
    /// @param value The 4-component value. Only first numComponents will be set.
    /// @param y The y coordinate in range of [0; height).
    void setPixel(unsigned x, unsigned y, glm::vec<4, Format_t> const &value);

    /// @brief Gets the pixel at (x, y).
    /// @param x The x coordinate in range of [0; width).
    /// @param y The y coordinate in range of [0; height).
    /// @return The value at the (x; y) absolute coordinates. Only first numComponents will be filled, the rest will be filled with 0.
    glm::vec<4, Format_t> getPixel(unsigned x, unsigned y) const;

    unsigned getWidth() const;
    unsigned getHeight() const;
    unsigned getNumComponents() const;
    /// @return glm::vec2{width, height}.
    glm::uvec2 getDimensions() const;
    Format_t const *getData() const;
    Format_t *getData();
};

/// @cond Doxygen_Suppress 
template <typename Format_t>
inline Bitmap<Format_t>::Bitmap(unsigned width, unsigned height, unsigned numComponents, Format_t const *src) : m_width(width), m_height(height), m_numComponents(numComponents)
{
    assert(m_numComponents <= 4 && "Components > 4 is yet not supported!");
    m_data.resize(width * height * numComponents);
    if(src) {
        std::copy(src, src + m_data.size(), m_data.begin());
    }
}
template <typename Format_t>
inline void Bitmap<Format_t>::setPixel(unsigned x, unsigned y, glm::vec<4, Format_t> const &value)
{
    assert(x < m_width && y < m_height && "x or y is out of range!");
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    Format_t *data = m_data.data();
    size_t offset = getOffsetOf(x, y);
    if (m_numComponents > 0) data[offset + 0] = value.x;
    if (m_numComponents > 1) data[offset + 1] = value.y;
    if (m_numComponents > 2) data[offset + 2] = value.z;
    if (m_numComponents > 3) data[offset + 3] = value.w;
}
template <typename Format_t>
inline glm::vec<4, Format_t> Bitmap<Format_t>::getPixel(unsigned x, unsigned y) const
{
    assert(x < m_width && y < m_height && "x or y is out of range!");
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    Format_t const *data = m_data.data();
    size_t offset = getOffsetOf(x, y);
    return glm::vec4(
        m_numComponents > 0 ? data[offset + 0] : 0.0f,
        m_numComponents > 1 ? data[offset + 1] : 0.0f,
        m_numComponents > 2 ? data[offset + 2] : 0.0f,
        m_numComponents > 3 ? data[offset + 3] : 0.0f
    );
}
template <typename Format_t> 
inline unsigned Bitmap<Format_t>::getWidth() const 
{ 
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    return m_width; 
}
template <typename Format_t> 
inline unsigned Bitmap<Format_t>::getHeight() const 
{ 
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    return m_height; 
}
template <typename Format_t> 
inline unsigned Bitmap<Format_t>::getNumComponents() const 
{ 
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    return m_numComponents; 
}
template <typename Format_t> 
inline glm::uvec2 Bitmap<Format_t>::getDimensions() const 
{ 
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    return glm::vec2{getWidth(), getHeight()}; 
}
template <typename Format_t> 
inline Format_t const *Bitmap<Format_t>::getData() const 
{ 
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    return m_data.data(); 
}
template <typename Format_t> 
inline Format_t *Bitmap<Format_t>::getData() 
{ 
    assert(m_data.size() == m_height * m_width * m_numComponents && "Bitmap not initialized!");
    return m_data.data(); 
}
/// @endcond
