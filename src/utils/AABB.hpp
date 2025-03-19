#pragma once
#include "glm/glm.hpp"

class AABB {
private:
public:
    glm::vec3 min = glm::vec3{0, 0, 0};
    glm::vec3 max = glm::vec3{0, 0, 0};

    inline void growToInclude(glm::vec3 const &point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
};