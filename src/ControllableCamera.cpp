#include "ControllableCamera.hpp"

const float sensitivitySlow = 0.05f;
const float movementSlow = 1.0f;

ControllableCamera::ControllableCamera(GLFWwindow *window, glm::vec3 const &position, glm::vec3 const &rotation) :
    Camera(position, rotation), 
    window(window) 
{}
ControllableCamera::~ControllableCamera() = default;

void ControllableCamera::update(double deltatime) {
    Camera::update();
    processPosition(deltatime);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    processRotation(xpos, ypos);
}

void ControllableCamera::processPosition(double deltatime) {
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += cameraSpeed * getFront() * (float) deltatime * movementSlow;
    } 
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= cameraSpeed * glm::cross(getFront(), getUp()) * (float) deltatime * movementSlow;
    } 
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= cameraSpeed * getFront() * (float) deltatime * movementSlow;
    } 
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += cameraSpeed * glm::cross(getFront(), getUp()) * (float) deltatime * movementSlow;
    }
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position += cameraSpeed * getUp() * (float) deltatime * movementSlow;
    }
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position += cameraSpeed * -getUp() * (float) deltatime * movementSlow;
    }
}
void ControllableCamera::processRotation(double xpos, double ypos) {
    if(!mouseLocked) return;

    if (firstCursorMove) {
        prevx = xpos;
        prevy = ypos;
        firstCursorMove = false;
    }

    float xoffset = xpos - prevx;
    float yoffset = prevy - ypos; // reversed since y-coordinates go from bottom to top

    prevx = xpos;
    prevy = ypos;

    xoffset *= sensitivity * sensitivitySlow;
    yoffset *= sensitivity * sensitivitySlow;

    rotation.x += xoffset;
    rotation.y += yoffset;

    if (rotation.y > 89.99f)
        rotation.y = 89.99f;
    if (rotation.y < -89.99f)
        rotation.y = -89.99f;
}
void ControllableCamera::processScroll(int yoffset) {
    fov -= (float) yoffset;
    if(fov < 1.0f) fov = 1.0f; 
    if(fov > 45.0f) fov = 45.0f; 
}