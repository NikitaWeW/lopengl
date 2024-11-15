#pragma once
#include "GLFW/glfw3.h"
#include "utils/Model.hpp"
#include "opengl/Texture.hpp"
#include <stdexcept>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
/*
Application state + initialisation / destruction
*/
struct Application {
public:
    GLFWwindow *window;
    glm::vec<2, int> windowSize;
    glm::vec3 clearColor = glm::vec3{0.0f};
    double deltatime = 0;
    unsigned frameCounter = 0;
    glm::vec3 cuberotation{0.1, 0.2, -0.1};

    Model *currentModel = nullptr;
    int currentModelIndex = 0; 
    std::vector<Model> models;
    std::vector<std::string> modelNames;
    char loadModelBuffer[1024];

    Texture *currentTexture = nullptr;
    int currentTextureIndex = 0;
    std::vector<Texture> textures;
    std::vector<std::string> textureNames;
    char loadTextureBuffer[1024];
    Application();
    ~Application();
    void addModel(char const *filepath);
    void addTexture(char const *filepath);
};
