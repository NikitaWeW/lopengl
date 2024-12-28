/*
all shitty bad initialization/uninitialization goes here.
this is really bad code, that is only use to test features. fame frog.
*/

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "logger.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"

#include <algorithm>
#include "Application.hpp"

OpenGlError Application::openglError;
extern const bool debug;

void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data)
{
    Application::openglError.id = id;
    Application::openglError.msg = msg;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
        Application::openglError.source = "api";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        Application::openglError.source = "window system";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        Application::openglError.source = "shader compiler";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        Application::openglError.source = "third party";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        Application::openglError.source = "application";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        Application::openglError.source = "unknown";
        break;

        default:
        Application::openglError.source = "unknown";
        break;
    }
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        Application::openglError.type = "error";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        Application::openglError.type = "deprecated behavior warning";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        Application::openglError.type = "udefined behavior warning";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        Application::openglError.type = "portability warning";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        Application::openglError.type = "performance warning";
        break;

        case GL_DEBUG_TYPE_OTHER:
        Application::openglError.type = "other message";
        break;

        case GL_DEBUG_TYPE_MARKER:
        Application::openglError.type = "marker message";
        break;

        default:
        Application::openglError.type = "unknown message";
        break;
    }
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        Application::openglError.severity = "high severity";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        Application::openglError.severity = "medium severity";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        Application::openglError.severity = "low severity";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        Application::openglError.severity = "notification";
        break;

        default:
        Application::openglError.severity = "unknown severity message";
        break;
    }

    LOG_WARN("%d: opengl %s %s, raised from %s:\n\t%s", 
            Application::openglError.id, 
            Application::openglError.severity.c_str(), 
            Application::openglError.type.c_str(), 
            Application::openglError.source.c_str(), 
            Application::openglError.msg.c_str());
}
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
Application::Application()
{
    logger_initConsoleLogger(stdout);
    logger_setLevel(debug ? LogLevel_DEBUG : LogLevel_INFO);
    if (!glfwInit()) {
        LOG_FATAL("failed to init glfw!");
        throw std::runtime_error("failed to init glfw!");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    GLFWvidmode const *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    window = glfwCreateWindow(mode->width * 0.7, mode->height * 0.9, "opengl", nullptr, nullptr);

    if (!window) {
        LOG_FATAL("failed to initialise window.");
        throw std::runtime_error("failed to initialise");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    if (!gladLoadGL((GLADloadfunc) glfwGetProcAddress)) {
        LOG_FATAL("gladLoadGL: Failed to initialize GLAD!");
        throw std::runtime_error("failed to initialise");
    }
    
    ImGui::CreateContext();
    IMGUI_CHECKVERSION();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    if(getenv("WAYLAND_DISPLAY")) LOG_INFO("wayland detected! imgui multiple viewports feature is not supported!");
    else io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
    ImGui::StyleColorsDark();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugCallback, nullptr);
    strcpy(loadModelBuffer, "");
    strcpy(loadTextureBuffer, "");
    LOG_DEBUG("running in debug mode!");
}
Application::~Application()
{
    LOG_INFO("cleaning up.");
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::loadModel(char const *filepath, LoadModelQuery query)
{
    std::string newFilepath{filepath};
    std::replace_if(newFilepath.begin(), newFilepath.end(), [](char c){ return c == '\\'; }, '/');
    try {
        models.push_back(Model{newFilepath, query.flipTextures, query.flipWindingOrder});
    } catch(std::runtime_error &e) {
        LOG_ERROR("%s", e.what());
    }
    currentModelIndex = models.size() - 1;
}
void Application::loadTexture(char const *filepath, LoadTextureQuery query)
{
    try {
        textures.push_back(Texture{filepath, query.flipTexture});
    } catch(std::runtime_error &e) {
        LOG_ERROR("%s", e.what());
    }
    currentTextureIndex = textures.size() - 1;
}
