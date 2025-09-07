#pragma once

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_stdlib.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include "core/equirect.hpp"
#include "tiny_obj_loader.h"
#include "core/ease_functions.hpp"

#include "core/ogl/Framebuffer.hpp"
#include "core/ogl/Texture.hpp"
#include "core/ogl/IndexBuffer.hpp"
#include "core/ogl/VertexBuffer.hpp"
#include "core/ogl/Shader.hpp"
#include "core/load.hpp"
#include "core/VelocityValue.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <random>
#include <limits>

#ifdef USE_RENDERDOC
#include <renderdoc_app.h>

// renderdoc in-app api
#ifdef __linux__
#include <dlfcn.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#endif

using seed_t = unsigned;

// for a small application like this i think its fine to use a single struct as an app state
struct Data
{
    GLFWwindow *window = nullptr;

    glm::ivec2 windowSize{-1};
    glm::dvec2 mousePos{0};
    glm::dvec2 prevMousePos{0};
    glm::vec2  deltaMouse{0};

    glm::mat4 viewMat{1.0f};
    glm::mat4 projMat{1.0f};

    glm::vec3 cameraPos{0};
    glm::vec3 cameraDir{1};

    ogl::Cubemap skybox;

    ogl::Cubemap texture;

    ogl::ShaderProgram cubeDrawShader;
    ogl::ShaderProgram cubeGenerateShader;
    ogl::ShaderProgram displayShader;
    ogl::ShaderProgram skyboxDrawShader;
    ogl::ShaderProgram gridShader;

    std::random_device dev;
    std::mt19937 gen{dev()};
    std::uniform_int_distribution<seed_t> dist{0, std::numeric_limits<seed_t>::max()};

    model::Mesh *model;
    model::Mesh cube;
    model::Mesh sphere;

    ogl::Framebuffer mainFBO;
    ogl::Renderbuffer mainRBO;
    ogl::TextureMS mainColor;

    ogl::Framebuffer displayFBO;
    ogl::Renderbuffer displayRBO;
    ogl::Texture displayTexture;

    float deltatime = 0.1;

    VelocityValue<glm::vec2> yawPitch{.value = glm::vec2{0}};
    VelocityValue<float> distance{.value = 3};

    #ifdef USE_RENDERDOC
    RENDERDOC_API_1_1_2 *rdoc_api = NULL;
    #endif

    struct Inputs {
        float sensitivity = 1;
        seed_t seed;
        unsigned textureSize = 100;
    bool spherical = false;
    } inputs;
};

constexpr unsigned NUM_SAMPLES = 4;
constexpr std::string_view CONFIG_WINDOW_NAME = "Properties";
constexpr float ZNEAR = 0.01;
constexpr float ZFAR = 100;
constexpr float CUBE_MODEL_SIZE = 1.0f; 
constexpr unsigned TEXTURE_FORMAT = GL_SRGB8;

bool init(Data &data);
void processInput(Data &data);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void drawFrame(Data &data);
void generateTexture(Data &data);
void randomSeed(Data &data);
void ui(Data &data);
void createTexture(Data &data);
