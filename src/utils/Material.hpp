#pragma once
#include "glm/glm.hpp"

struct Material {
    glm::vec3 ambient = glm::vec3{0};
    glm::vec3 diffuse = glm::vec3{0};
    glm::vec3 specular = glm::vec3{0};
    float shininess = 0;
};