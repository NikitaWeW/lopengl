#pragma once

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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

    ogl::ShaderProgram cubeDrawShader{"shaders/prop"};
    // ogl::ShaderProgram cubeGenerateShader{"shaders/prop"};
    ogl::ShaderProgram displayShader{"shaders/hdrImage"};
    ogl::ShaderProgram skyboxDrawShader{"shaders/skybox"};
    // ogl::ShaderProgram skyboxGenerateShader{"shaders/skybox"};
    ogl::ShaderProgram gridShader{"shaders/grid"};

    model::Mesh cube;

    ogl::Framebuffer mainFBO;
    ogl::Renderbuffer mainRBO{0};
    ogl::TextureMS mainColor{GL_LINEAR, GL_CLAMP_TO_EDGE};

    ogl::Framebuffer displayFBO;
    ogl::Renderbuffer displayRBO{0};
    ogl::Texture displayTexture{GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE};

    float deltatime = 0.1;

    VelocityValue<glm::vec2> yawPitch{.value = glm::vec2{0}};
    VelocityValue<float> distance{.value = 3};

    struct Inputs {
        float sensitivity = 1;
    } inputs;
};

constexpr unsigned NUM_SAMPLES = 4;
constexpr std::string_view CONFIG_WINDOW_NAME = "Properties";
constexpr float ZNEAR = 0.01;
constexpr float ZFAR = 100;
constexpr float CUBE_MODEL_SIZE = 1.0f; 

bool init(GLFWwindow **window);
void processInput(Data &data);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void drawFrame(Data &data);
