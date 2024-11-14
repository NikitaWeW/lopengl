#pragma once
#include "glm/glm.hpp"
#include <map>
#include <tuple>

class Camera {
private:
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 front;
    mutable std::map<std::tuple<int, int, float, float, float>, glm::mat4> m_ProjectionMatrixCache;
public:
    glm::vec3 position;
    glm::vec3 rotation;
    float far = 1000;
    float near = 0.01f;
    float fov;
public:
    Camera(glm::vec3 const &pos = glm::vec3(0), glm::vec3 const &rotation = glm::vec3(0));
    virtual ~Camera();
    virtual void update();
    virtual glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjectionMatrix(int const windowWidth, int const windowHeight) const;
    virtual inline glm::vec3 getFront() const {
        return front;
    }
    virtual inline glm::vec3 getRight() const {
        return right;
    }
    virtual inline glm::vec3 getUp() const {
        return up;
    }
};