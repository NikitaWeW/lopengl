#pragma once
#include "glm/glm.hpp"

struct Material {
    glm::vec3 color = {0, 0, 0};
    glm::vec3 emission = {0, 0, 0};
    float roughness = 1;
};