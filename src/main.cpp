/*
        +____________+
        /:\         ,:\
       / : \       , : \
      /  :  \     ,  :  \
     /   :   +-----------+
    +....:../:...+   :  /|
    |\   +./.:...`...+ / |
    | \ ,`/  :   :` ,`/  |
    |  \ /`. :   : ` /`  |
    | , +-----------+  ` |
    |,  |   `+...:,.|...`+
    +...|...,'...+  |   /
     \  |  ,     `  |  /
      \ | ,       ` | /
       \|,         `|/
        +___________+

2-Dimensional Representation Of A 3-Dimensional Cross-Section Of A 4-Dimensional Cube
*/

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "logger.h"
#include "tiny_obj_loader.h"

#include "opengl/Framebuffer.hpp"
#include "opengl/Texture.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/Shader.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
#include <stdexcept>

struct Mesh
{
    ogl::VertexBuffer vbo;
    ogl::VertexArray vao;
    ogl::IndexBuffer ibo;
    unsigned count;
};

bool init(GLFWwindow **window);
Mesh load(std::string_view path, bool flip = false);

int main(int argc, char **argv)
{
    GLFWwindow *window = nullptr;
    if(!init(&window)) {
        LOG_FATAL("failed to init!");
        return -1;
    }
    assert(window);

    // ===================================

    ogl::Cubemap skybox{"res/textures/qwantani_dawn_puresky_2k.hdr"};
    ogl::ShaderProgram displayShader{"shaders/display"};
    ogl::ShaderProgram skyboxShader{"shaders/skybox"};

    // ===================================

    glm::ivec2 windowDim{-1};
    float deltatime = 0.1; // seconds
    long long unsigned frameCounter = 0;
    bool cameraLocked = false;

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfwGetWindowSize(window, &windowDim.x, &windowDim.y);
        glfwSetInputMode(window, GLFW_CURSOR, cameraLocked ? GLFW_CURSOR_CAPTURED : GLFW_CURSOR_NORMAL);

        glm::mat4 projection = glm::perspective<float>(glm::radians(45.0f), windowDim.x / windowDim.y, 0.01, 100);

        glViewport(0, 0, windowDim.x, windowDim.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        skyboxShader.bind();
        skybox.bind(0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
        ++frameCounter;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}
void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *objMesh)
{
    if(source == GL_DEBUG_SOURCE_SHADER_COMPILER && (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_OTHER)) return; // handled by ShaderProgram class 

    struct OpenGlError {
        GLuint id;
        std::string source;
        std::string type;
        std::string severity;
        std::string msg;
    } error;
    
    error.id = id;
    error.msg = msg;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
        error.source = "api";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        error.source = "window system";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        error.source = "shader compiler";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        error.source = "third party";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        error.source = "application";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        error.source = "unknown";
        break;

        default:
        error.source = "unknown";
        break;
    }
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        error.type = "error";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        error.type = "deprecated behavior warning";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        error.type = "udefined behavior warning";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        error.type = "portability warning";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        error.type = "performance warning";
        break;

        case GL_DEBUG_TYPE_OTHER:
        error.type = "message";
        break;

        case GL_DEBUG_TYPE_MARKER:
        error.type = "marker message";
        break;

        default:
        error.type = "unknown message";
        break;
    }
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        error.severity = "high";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        error.severity = "medium";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        error.severity = "low";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        error.severity = "notification";
        break;

        default:
        error.severity = "unknown";
        break;
    }

    LOG_WARN("%d: opengl %s severity %s, raised from %s:\n\t%s", 
            error.id, 
            error.severity.c_str(), 
            error.type.c_str(), 
            error.source.c_str(), 
            error.msg.c_str());
}
bool init(GLFWwindow **window)
{
    if (!glfwInit()) {
        LOG_FATAL("failed to initialize glfw!");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    GLFWvidmode const *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    *window = glfwCreateWindow(mode->width * 0.7, mode->height * 0.9, "opengl", nullptr, nullptr);

    if (!*window) {
        LOG_FATAL("failed to initialize window.");
        return false;
    }
    glfwMakeContextCurrent(*window);
    if (!gladLoadGL((GLADloadfunc) glfwGetProcAddress)) {
        LOG_FATAL("gladLoadGL: Failed to initialize GLAD!");
        return false;
    }
    
    ImGui::CreateContext();
    IMGUI_CHECKVERSION();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    if(getenv("WAYLAND_DISPLAY")) LOG_INFO("wayland detected! imgui multiple viewports feature is not supported!");
    else io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplGlfw_InitForOpenGL(*window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
    ImGui::StyleColorsDark();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallback, nullptr);
    LOG_DEBUG("running in debug mode!");
    
    glfwSwapInterval(0);

    return true;
}
Mesh load(std::string_view path, bool flip)
{
    return Mesh{};
}
