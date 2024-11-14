#pragma once
#include "GLFW/glfw3.h"


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
class Application {
public:
    GLFWwindow *window;
    glm::vec<2, int> windowSize;
    Application();
    ~Application();
};
