#pragma once
#include "glm/glm.hpp"

/// @brief Axis aligned bounding box.
/// @tparam T The type of the bounding values.
template<typename T = glm::vec3>
struct AABB
{
    T min{0}; /// Minimal corner. 
    T max{0}; /// Maximal corner.

    AABB() = default;

    /// @brief Adjust @p min and @p max to include the point @p p.
    inline void growToInclude(T const &p)
    {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
};
