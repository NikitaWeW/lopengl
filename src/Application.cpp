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
#include "utils/ControllableCamera.hpp"

#include <algorithm>
#include "Application.hpp"

OpenGlError Application::openglError;
extern const bool debug;

void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data)
{
    if(source == GL_DEBUG_SOURCE_SHADER_COMPILER && (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_OTHER)) return; // handled by ShaderProgram class 

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
        Application::openglError.type = "message";
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
        Application::openglError.severity = "unknown severity";
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
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ControllableCamera &camera = *static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) camera.locked = !camera.locked;
    if (camera.locked) {
        camera.firstCursorMove = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if(key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
        // evaluate fov
        if (camera.fov < 1.0f)
            camera.fov = 1.0f;
        if (camera.fov > 45.0f)
            camera.fov = 45.0f;
    } else {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    ControllableCamera *cam = static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if(cam->locked) {
        cam->fov -= (float)yoffset * 4.5f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            if (cam->fov < 0.01f)
                cam->fov = 0.01f;
            if (cam->fov > 60.0f)
                cam->fov = 60.0f;
        } else {
            if (cam->fov < 1.0f)
                cam->fov = 1.0f;
            if (cam->fov > 45.0f)
                cam->fov = 45.0f;
        }
    } else {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    }
}