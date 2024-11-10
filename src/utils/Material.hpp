#pragma once
#include "glm/glm.hpp"

struct Material {
    glm::vec3 ambient =  glm::vec3{0.0f};
    glm::vec3 diffuse =  glm::vec3{0.0f};
    glm::vec3 specular = glm::vec3{0.0f};
    float shininess = 0.2f;
};