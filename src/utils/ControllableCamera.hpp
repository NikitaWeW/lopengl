#pragma once
#include "Camera.hpp"
#include "GLFW/glfw3.h"

class ControllableCamera : public Camera {
private:
    bool roatationCallback;
    double prevx;
    double prevy;
public:
    bool firstCursorMove = true;
    float sensitivity = 1.0f;
    float cameraSpeed = 1.0f;
    bool mouseLocked = false;
    GLFWwindow *window;

    ControllableCamera(GLFWwindow *window, glm::vec3 const &position = glm::vec3(0), glm::vec3 const &rotation = glm::vec3(0));
    virtual ~ControllableCamera();

    virtual void update(double deltatime);
    virtual void processPosition(double deltatime);
    virtual void processRotation(double xpos, double ypos);
    virtual void processScroll(int xoffset);
};

