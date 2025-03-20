#pragma once
#include "GLFW/glfw3.h"
#include "utils/Model.hpp"
#include "opengl/Texture.hpp"
#include <stdexcept>
#include <optional>
#include "utils/ControllableCamera.hpp"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
struct OpenGlError {
    GLuint id;
    std::string source;
    std::string type;
    std::string severity;
    std::string msg;
};

/*
 * Application state + RAII initialisation / destruction
 */
struct Application {
public:
    static OpenGlError openglError;
    std::string lastFailedShaderLog;
    std::string lastFailedShaderName;
    GLFWwindow *window;
    ControllableCamera camera;

    glm::vec3 clearColor = glm::vec3{0.0f};

    double deltatime = 0;
    unsigned frameCounter = 0;
    unsigned numAccumFrames = 0;

    std::vector<ShaderProgram> shaders;
    std::vector<unsigned> displayShaders;

    bool failedToReloadShaders = false;
public:
    Application();
    ~Application();
    bool reloadShaders();
};
