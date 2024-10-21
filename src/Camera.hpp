#pragma once
#include "glm/glm.hpp"

class Camera {
private:
    glm::vec3 Right;
    glm::vec3 Up;
    glm::vec3 Front;
public:
    glm::vec3 position;
    glm::vec3 rotation;

public:
    Camera(glm::vec3 pos = glm::vec3(0), glm::vec3 rotation = glm::vec3(0));
    void update();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(int windowWidth, int windowHeight) const;
};