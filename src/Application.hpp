#pragma once
#include "GLFW/glfw3.h"
#include "utils/Model.hpp"
#include "opengl/Texture.hpp"
#include <stdexcept>
#include <optional>
#include "scene/Scene.hpp"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
struct OpenGlError {
    GLuint id;
    std::string source;
    std::string type;
    std::string severity;
    std::string msg;
};
/*
Application state + RAII initialisation / destruction + model / texture loading UI
this is really bad code, that is only use to test features. fame frog.
*/
struct Application {
private:
    void loadModel(char const *filepath, std::optional<bool> flipTextures);
    void loadTexture(char const *filepath, std::optional<bool> flipTextures);
    std::map<std::string, bool> flipTexturesMap;
    std::map<std::string, unsigned> windingMap;
public:
    static OpenGlError openglError;
    GLFWwindow *window;

    Shader plainColorShader;
    Model quad;
    Model cube;

    glm::vec3 outlineColor = glm::vec3{0.4, 0.4, 0.0};
    glm::vec3 clearColor = glm::vec3{0.0f};
    glm::vec3 currentModelPosition = glm::vec3{0};
    glm::vec3 currentModelRotation = glm::vec3{0};
    glm::vec3 currentModelScale = glm::vec3{1};

    // glm::vec3 currentModelPosition = glm::vec3{0.0f};
    // glm::vec3 currentModelRotation = glm::vec3{0.0f};
    // glm::vec3 currentModelScale    = glm::vec3{1.0f};

    double deltatime = 0;
    unsigned frameCounter = 0;
    unsigned updateCounter = 0;

    Texture *currentTexture = nullptr; // i know that could be deleted, and be accessed via textures[currentTextureIndex], maybe i will fix this in the future who knows
    Model *currentModel = nullptr;

    int currentModelIndex = 0; 
    int currentShaderIndex = 0;
    int currentTextureIndex = 0;
    int currentSceneIndex = 0;
    int windingIndex = 0;

    std::vector<Model> models;
    std::vector<Shader> shaders;
    std::vector<Texture> textures;
    std::vector<Scene *> scenes; // because of virtual functions, i need to store the reference of base class with derived data (makes sence)

    std::vector<std::string> textureNames;
    std::vector<std::string> modelNames;

    char loadModelBuffer[1024];
    char loadTextureBuffer[1024];

    bool wireframe = false;
    bool flashlight = true;
    bool objectOutline = false;
    bool sceneControlsInSeparateWindow = false;
    bool flipTextures = false;
public:
    Application();
    ~Application();
    void addModel(char const *filepath, unsigned winding, bool loadNow = false, std::optional<bool> flipTextures = {});
    void addTexture(char const *filepath, bool loadNow = false, std::optional<bool> flipTextures = {});
    void applyModel();
    void applyTexture();
};
