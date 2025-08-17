#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <map>
#include <tuple>
#include "GLFW/glfw3.h"

class Camera {
private:
    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
public:
    glm::quat orientation = glm::angleAxis(0.0f, glm::vec3{0, 0, -1});
    glm::vec3 position;
    float far = 100;
    float near = 0.01f;
    float fov = 45;
    int width = 0; 
    int height = 0;
    float sensitivity = 1;
public:
    Camera() = default;
    virtual ~Camera();
    virtual void update(double deltatime);
    virtual glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjectionMatrix() const;
    
    virtual inline glm::vec3 getFront() const { return m_forward; }
    virtual inline glm::vec3 getRight() const { return m_right; }
    virtual inline glm::vec3 getUp()    const { return m_up;    }
};

class ControllableCamera : public Camera {
private:
    double prevx;
    double prevy;
public:
    bool firstCursorMove = true;
    float sensitivity = 1.0f;
    float speed = 7.0f;
    bool locked = false;
    GLFWwindow *window;

    ControllableCamera(GLFWwindow *window);
    ControllableCamera() = default;
    virtual ~ControllableCamera() = default;

    virtual void update(double deltatime);
    virtual void processPosition(double deltatime);
    virtual void processRotation(double xpos, double ypos, float deltatime);
    virtual void processScroll(int xoffset);
};

