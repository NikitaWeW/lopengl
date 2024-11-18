#pragma once
#include "glm/glm.hpp"

struct Light {
    glm::vec3 position = glm::vec3{0};

    glm::vec3 ambient = glm::vec3{0};
    glm::vec3 diffuse = glm::vec3{1};
    glm::vec3 specular = glm::vec3{0};

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;
};

struct SpotLight {
    glm::vec3 position = glm::vec3{0};
    glm::vec3 direction = glm::vec3{0, 0, -1};
    float innerCutoff = glm::cos(glm::radians(20.0f));
    float outerCutoff = glm::cos(glm::radians(40.0f));

    glm::vec3 ambient = glm::vec3{0};
    glm::vec3 diffuse = glm::vec3{1};
    glm::vec3 specular = glm::vec3{0};

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;
};