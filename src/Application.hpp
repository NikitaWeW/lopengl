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
to organise loading ¯⁠\⁠_⁠(⁠ツ⁠)⁠_⁠/⁠¯
*/
struct LoadModelQuery {
    bool flipTextures = false;
    bool flipWindingOrder = false;
};
struct LoadTextureQuery {
    bool flipTexture = false;
};
/*
Application state + RAII initialisation / destruction + model / texture loading UI
this is really bad code, that is only use to test features. fame frog.
*/
struct Application {
public:
    static OpenGlError openglError;
    GLFWwindow *window;

    Shader plainColorShader;
    Model quad;
    Model cube;

    glm::vec3 outlineColor = glm::vec3{0.4, 0.4, 0.0};
    glm::vec3 clearColor = glm::vec3{0.0f};

    // glm::vec3 currentModelPosition = glm::vec3{0.0f};
    // glm::vec3 currentModelRotation = glm::vec3{0.0f};
    // glm::vec3 currentModelScale    = glm::vec3{1.0f};

    double deltatime = 0;
    unsigned frameCounter = 0;
    unsigned updateCounter = 0;

    int currentModelIndex = 0; 
    int currentShaderIndex = 0;
    int currentTextureIndex = 0;
    int currentSceneIndex = 0;

    std::vector<Model> models;
    std::vector<Shader> shaders;
    std::vector<Texture> textures;

    char loadModelBuffer[1024];
    char loadTextureBuffer[1024];

    bool wireframe = false;
    bool flashlight = true;
    bool objectOutline = false;
    bool sceneControlsInSeparateWindow = false;
    bool flipTextures = false;
    bool flipWinding = false;
public:
    Application();
    ~Application();

    void loadModel(char const *filepath, LoadModelQuery query);
    void loadTexture(char const *filepath, LoadTextureQuery query);
};
