#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <logger.h>
#include <stdexcept>

#include "Application.hpp"
#include "Renderer.hpp"

extern const bool debug;


void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    GLCALL(glViewport(0, 0, width, height));
}
Application::Application() {
    logger_initConsoleLogger(stdout);
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

    GLCALL(glEnable(GL_BLEND));
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}
Application::~Application() {
    LOG_INFO("cleaning up.");
    glfwDestroyWindow(window);
    glfwTerminate();
}