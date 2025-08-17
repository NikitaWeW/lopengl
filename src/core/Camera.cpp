#include "Camera.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

Camera::~Camera() {}

void Camera::update(double deltatime) 
{
    glm::mat4 const &invViewMat = glm::inverse(getViewMatrix());
    m_forward = glm::normalize(glm::vec3{invViewMat * glm::vec4{0, 0,-1, 0}});
    m_right   = glm::normalize(glm::vec3{invViewMat * glm::vec4{1, 0, 0, 0}});
    m_up      = glm::normalize(glm::vec3{invViewMat * glm::vec4{0, 1, 0, 0}});
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::translate(glm::mat4_cast(glm::normalize(orientation)), -position);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::mat4{glm::perspective(glm::radians(fov), (float) width / height, near, far)};
}

const float sensitivitySlow = 0.05f;

ControllableCamera::ControllableCamera(GLFWwindow *window) :
    window(window) 
{}

void ControllableCamera::update(double deltatime) {
    Camera::update(deltatime);
    processPosition(deltatime);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    processRotation(xpos, ypos, static_cast<float>(deltatime));

    glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void ControllableCamera::processPosition(double deltatime) {
    if(!locked) return;

    float cameraspeed = speed * (float) deltatime;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += getFront() * cameraspeed;
    } 
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= glm::cross(getFront(), getUp()) * cameraspeed;
    } 
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= getFront() * cameraspeed;
    } 
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += glm::cross(getFront(), getUp()) * cameraspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position += getUp() * cameraspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position += -getUp() * cameraspeed;
    }
}
void ControllableCamera::processRotation(double xpos, double ypos, float deltatime) {
    if(!locked) return;

    if (firstCursorMove) {
        prevx = xpos;
        prevy = ypos;
        firstCursorMove = false;
    }

    float xoffset = xpos - prevx;
    float yoffset = ypos - prevy;

    prevx = xpos;
    prevy = ypos;

    xoffset *= sensitivity * sensitivitySlow;
    yoffset *= sensitivity * sensitivitySlow;

    float rollOffset = 0;
    
    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        rollOffset -= sensitivity * 100 * deltatime;
    } if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        rollOffset += sensitivity * 100 * deltatime;
    } if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        orientation = glm::quat{1, 0, 0, 0};
    }

    glm::quat yaw   = glm::angleAxis(glm::radians(xoffset),   glm::vec3{0, 1, 0});
    glm::quat pitch = glm::angleAxis(glm::radians(yoffset),   glm::vec3{1, 0, 0});
    glm::quat roll  = glm::angleAxis(glm::radians(rollOffset), glm::vec3{0, 0, -1});

    orientation = glm::normalize(roll * pitch * orientation * yaw);
}
void ControllableCamera::processScroll(int yoffset) {
    fov -= (float) yoffset;
    if(fov < 1.0f) fov = 1.0f; 
    if(fov > 45.0f) fov = 45.0f; 
}