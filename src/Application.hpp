#pragma once
#include <GLFW/glfw3.h>

extern GLFWwindow* window;
extern glm::vec<2, int> windowSize;
extern size_t iteration;
extern double frameBeginTimeSeconds;
extern double renderTimeSeconds;
extern double deltaTime;
extern double FPS;


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
class Application {
public:
    Application();
    ~Application();
};
