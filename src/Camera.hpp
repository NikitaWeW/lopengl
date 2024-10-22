#pragma once
#include "glm/glm.hpp"

class Camera {
private:
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 front;
public:
    glm::vec3 position;
    glm::vec3 rotation;
public:
    Camera(glm::vec3 pos = glm::vec3(0), glm::vec3 rotation = glm::vec3(0));
    void update();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(int windowWidth, int windowHeight) const;
    inline glm::vec3 getFront() const {
        return front;
    }
    inline glm::vec3 getRight() const {
        return right;
    }
    inline glm::vec3 getUp() const {
        return up;
    }
};