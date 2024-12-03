#pragma once
#include "GLFW/glfw3.h"
#include "utils/Model.hpp"
#include "opengl/Texture.hpp"
#include <stdexcept>
#include <optional>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
struct OpenGlError {
    GLuint id;
    std::string source;
    std::string type;
    std::string severity;
    std::string msg;
};
/*
Application state + RAII initialisation / destruction
this is really bad code, that is only use to test features. fame frog.
*/
struct Application {
private:
    void loadModel(char const *filepath);
    void loadTexture(char const *filepath);
public:
    static OpenGlError openglError;
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
    bool flashlight = true;

    int currentShaderIndex = 0;
    std::vector<Shader> shaders;

public:
    Application();
    ~Application();
    void addModel(char const *filepath, bool loadNow = false);
    void addTexture(char const *filepath, bool loadNow = false);
    void applyModel();
    void applyTexture();
};
