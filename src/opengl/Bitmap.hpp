#pragma once
#include "glm/glm.hpp"
#include <vector>

template <typename Format_t = float>
class Bitmap
{
private:
    unsigned m_width, m_height, m_numComponents;
    std::vector<Format_t> m_data;

    inline size_t getOffsetOf(unsigned x, unsigned y) const { return m_numComponents * (y * m_width + x); }
public:
    Bitmap() = default;
    Bitmap(unsigned width, unsigned height, unsigned numComponents, Format_t const *data = nullptr);

    void setPixel(unsigned x, unsigned y, glm::vec<4, Format_t> const &value);
    glm::vec<4, Format_t> getPixel(unsigned x, unsigned y) const;

    inline unsigned getWidth() const { return m_width; }
    inline unsigned getHeight() const { return m_height; }
    inline unsigned getNumComponents() const { return m_numComponents; }
    inline glm::vec2 getDimensions() const { return glm::vec2{getWidth(), getHeight()}; }
    inline Format_t const *getData() const { return m_data.data(); }
    inline Format_t *getData() { return m_data.data(); }
};

template <typename Format_t>
inline Bitmap<Format_t>::Bitmap(unsigned width, unsigned height, unsigned numComponents, Format_t const *src) : m_width(width), m_height(height), m_numComponents(numComponents)
{
    assert(m_numComponents <= 4);
    m_data.resize(width * height * numComponents);
    if(src) {
        std::copy(src, src + m_data.size(), m_data.begin());
    }
}

template <typename Format_t>
inline void Bitmap<Format_t>::setPixel(unsigned x, unsigned y, glm::vec<4, Format_t> const &value)
{
    assert(x < m_width && y < m_height);
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
    assert(x < m_width && y < m_height);
    Format_t const *data = m_data.data();
    size_t offset = getOffsetOf(x, y);
    return glm::vec4(
        m_numComponents > 0 ? data[offset + 0] : 0.0f,
        m_numComponents > 1 ? data[offset + 1] : 0.0f,
        m_numComponents > 2 ? data[offset + 2] : 0.0f,
        m_numComponents > 3 ? data[offset + 3] : 0.0f
    );
}
