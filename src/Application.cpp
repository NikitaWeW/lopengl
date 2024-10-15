#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <logger.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "Application.hpp"
#include "Renderer.hpp"
#include <fstream>
#include <filesystem>

extern const bool debug;

GLFWwindow* window;
glm::vec<2, int> windowSize;

size_t iteration = 0;
double frameBeginTimeSeconds;
double renderTimeSeconds = 0;
double deltaTime = 0;
double FPS = 0;
double renderFPS = 0;

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    windowSize.x = width;
    windowSize.y = height;
    GLCALL(glViewport(0, 0, width, height));
}
Application::Application() {
    std::filesystem::create_directory("logs");
    
    logger_initConsoleLogger(stdout);
    logger_initFileLogger("logs/main.log", 1024 * 1024, 5);
    logger_setLevel(debug ? LogLevel_DEBUG : LogLevel_INFO);
    if (glfwInit()) LOG_INFO("glfw initialised successfully");
    else {
        LOG_FATAL("failed to init glfw!");
        throw std::runtime_error("failed to init glfw!");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(640, 480, "-- FPS", NULL, NULL);
    if (window) LOG_INFO("window initialised successfully at adress %i", window);
    else {
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

    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

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
}
Application::~Application() {
    LOG_INFO("cleaning up.");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}