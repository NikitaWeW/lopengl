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

#include <algorithm>
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

    LOG_ERROR("%d: %s of %s severity, raised from %s: %s", id, &_type.at(0), &_severity.at(0), &_source.at(0), msg);
    if(type == GL_DEBUG_TYPE_ERROR && severity == GL_DEBUG_SEVERITY_HIGH) throw std::logic_error("opengl high severity error");
}
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
void Application::loadModel(char const *filepath)
{
    std::string newFilepath{filepath};
    std::replace_if(newFilepath.begin(), newFilepath.end(), [](char c){ return c == '\\'; }, '/');
    LOG_INFO("loading model \"%s\"...", filepath);
    Model model{newFilepath};
    LOG_INFO("model loaded!");
    models.push_back(std::move(model));
    currentModel = &*models.rbegin();
    currentModelIndex = models.size() - 1;
}
void Application::loadTexture(char const *filepath)
{
    LOG_INFO("loading texture \"%s\"...", filepath);
    Texture texture{filepath};
    LOG_INFO("texture loaded!");
    textures.push_back(texture);
    currentTexture = &*textures.rbegin(); 
    currentTextureIndex = models.size() - 1;
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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLDebugMessageCallback, nullptr);
    strcpy(loadModelBuffer, "");
    strcpy(loadTextureBuffer, "");
    LOG_DEBUG("running in debug mode!");
}
Application::~Application() {
    LOG_INFO("cleaning up.");
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::addModel(char const * filepath, bool loadNow)
{
    try {
        if(loadNow) {
            loadModel(filepath);
        }
        modelNames.push_back({filepath});
    } catch(std::runtime_error &e) {
        LOG_ERROR("%s", e.what());
    }
}
void Application::addTexture(char const * filepath, bool loadNow)
{
    try {
        if(loadNow) {
            loadTexture(filepath);
        }
        textureNames.push_back({filepath});
    } catch(std::runtime_error &e) {
        LOG_ERROR("failed to add texture: %s", e.what());
    }
}

void Application::applyModel()
{
    auto model = std::find_if(models.begin(), models.end(), [this](Model const &model){ return model.getFilepath() == modelNames.at(currentModelIndex); });
    if(model == models.end()) {
        loadModel(modelNames.at(currentModelIndex).c_str());
    } else {
        currentModel = &*model;
    }
}
void Application::applyTexture()
{
    auto texture = std::find_if(textures.begin(), textures.end(), [this](Texture const &texture){ return texture.getFilePath() == textureNames.at(currentModelIndex); });
    if(texture == textures.end()) {
        loadModel(textureNames.at(currentModelIndex).c_str());
    } else {
        currentTexture = &*texture;
    }
}