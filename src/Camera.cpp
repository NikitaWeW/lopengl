#include "Camera.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(glm::vec3 pos, glm::vec3 rotation) : position(pos), rotation(rotation)
{
}

void Camera::update()
{
    Front = glm::normalize(glm::vec3(
        cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y)),
        sin(glm::radians(rotation.y)),
        sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.y))
    ));
    Right = glm::normalize(glm::cross(Front, glm::vec3(0, 1, 0)));
    Up    = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(int windowWidth, int windowHeight) const
{
    return glm::mat4(glm::perspective(45.0f, (float) windowWidth / windowHeight, 0.001f, -100.0f));
}
