/*
all shitty bad initialization/uninitialization goes here
*/

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "logger.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include <stdexcept>

#include "Application.hpp"

extern const bool debug;

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar *msg, const void *data)
{
    std::string_view _source;
    std::string_view _type;
    std::string_view _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
        _source = "api";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "window system";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "shader compiler";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "third party";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        _source = "application";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        _source = "unknown";
        break;

        default:
        _source = "unknown";
        break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        _type = "error";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "deprecated behavior";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "udefined behavior";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        _type = "portability";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "performance";
        break;

        case GL_DEBUG_TYPE_OTHER:
        _type = "other";
        break;

        case GL_DEBUG_TYPE_MARKER:
        _type = "marker";
        break;

        default:
        _type = "unknown";
        break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        _severity = "high";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "medium";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        _severity = "low";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "notification";
        break;

        default:
        _severity = "unknown";
        break;
    }

    LOG_ERROR("%d: %s of %s severity, raised from %s: %s", id, _type, _severity, _source, msg);
    if(type == GL_DEBUG_TYPE_ERROR && severity == GL_DEBUG_SEVERITY_HIGH) throw std::logic_error("opengl high severity error");
}
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
Application::Application() {
    logger_initConsoleLogger(stdout);
    logger_setLevel(debug ? LogLevel_DEBUG : LogLevel_INFO);
    if (!glfwInit()) {
        LOG_FATAL("failed to init glfw!");
        throw std::runtime_error("failed to init glfw!");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window = glfwCreateWindow(640, 480, "opengl", nullptr, nullptr);
    if (!window) {
        LOG_FATAL("failed to initialise window.");
        throw std::runtime_error("failed to initialise window.");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    if (gladLoadGL((GLADloadfunc) glfwGetProcAddress)) 
        LOG_INFO("gl loaded successfully. gl version: %s", glGetString(GL_VERSION));
    else{
        LOG_FATAL("gladLoadGL: Failed to initialize GLAD!");
        throw std::runtime_error("gladLoadGL: Failed to initialize GLAD!");
    }
    
    ImGui::CreateContext();
    IMGUI_CHECKVERSION();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLDebugMessageCallback, nullptr);
}
Application::~Application() {
    LOG_INFO("cleaning up.");
    glfwDestroyWindow(window);
    glfwTerminate();
}