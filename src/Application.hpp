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
private:
    void loadModel(char const *filepath);
    void loadTexture(char const *filepath);
public:
    GLFWwindow *window;
    glm::vec3 clearColor = glm::vec3{0.0f};
    double deltatime = 0;
    unsigned frameCounter = 0;
    unsigned updateCounter = 0;
    glm::vec3 cuberotation{0};

    Model *currentModel = nullptr;
    int currentModelIndex = 0; 
    std::vector<Model> models;
    std::vector<std::string> modelNames;
    char loadModelBuffer[1024];

    Texture *currentTexture = nullptr;
    int currentTextureIndex = textureNames.size() - 1;
    std::vector<Texture> textures;
    std::vector<std::string> textureNames;
    char loadTextureBuffer[1024];

    bool wireframe = false;
    // bool moveLight = true;
    // glm::vec3 lightPosBegin = glm::vec3{-1, 1, 1.5f};
    // glm::vec3 lightPosEnd = glm::vec3{1.5f, -1, 3};
    // float lightSpeed = 0.005f;


    Application();
    ~Application();
    void addModel(char const *filepath, bool loadNow = false);
    void addTexture(char const *filepath, bool loadNow = false);
    void applyModel();
    void applyTexture();
};
