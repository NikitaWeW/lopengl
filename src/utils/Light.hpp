#pragma once
#include "glm/glm.hpp"

struct Light {
    glm::vec3 position = glm::vec3{0.0f};
    glm::vec3 ambient =  glm::vec3{0.0f};
    glm::vec3 diffuse =  glm::vec3{0.0f};
    glm::vec3 specular = glm::vec3{0.0f};
};