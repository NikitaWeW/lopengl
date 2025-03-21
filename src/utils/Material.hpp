#pragma once
#include "glm/glm.hpp"

struct Material {
    glm::vec3 color = {0, 0, 0};
    glm::vec3 emission = {0, 0, 0};
    float smoothness = 0;
    float specularProbability = 0;
    glm::vec3 specularColor = {1, 1, 1};
};