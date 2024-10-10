#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm.hpp>
#include <logger.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "Application.hpp"
#include "render.hpp"

#include <iostream> //standart includes
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

float const vertices[] = {
    -0.5f, -0.5f, 0.0f, // 0
     0.5f, -0.5f, 0.0f, // 1
     0.5f,  0.5f, 0.0f, // 2
    -0.5f,  0.5f, 0.0f, // 3
};
unsigned const indicies[] = {
    0, 1, 2,
    0, 2, 3
};

ShaderProgram shaderProg;


void GLErrorCallback(GLenum error) {
    char const *errorString;
    switch (error) {
        case GL_NO_ERROR:
            errorString = "No error";
        case GL_INVALID_ENUM:
            errorString = "Invalid enum";
        case GL_INVALID_VALUE:
            errorString = "Invalid value";
        case GL_INVALID_OPERATION:
            errorString = "Invalid operation";
        case GL_STACK_OVERFLOW:
            errorString = "Stack overflow";
        case GL_STACK_UNDERFLOW:
            errorString = "Stack underflow";
        case GL_OUT_OF_MEMORY:
            errorString = "Out of memory";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = "Invalid framebuffer operation";
        default:
            errorString = "Unknown error";
    }
    LOG_ERROR("opengl error: %s", errorString);
}

double getTimeSeconds() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count(); 
}
int main()
{
    Application app;
    ImGuiIO const &io = ImGui::GetIO();
    shaderProg.ParceShaderFile("src/basic.glsl");
    if(!shaderProg.CompileShaders()) {
        LOG_FATAL("failed to compile shaders.");
        return -1;
    }
    GLCALL(glUseProgram(shaderProg.ShaderProgramID));

    VertexBuffer VB(vertices, sizeof(vertices)); // already bound


    unsigned VAO;
    GLCALL(glGenVertexArrays(1, &VAO));
    GLCALL(glBindVertexArray(VAO));

    unsigned EBO;
    GLCALL(glGenBuffers(1, &EBO));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), indicies, GL_STATIC_DRAW));

    GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));
    GLCALL(glEnableVertexAttribArray(0));

    GLCALL(glClearColor(0, 0.1f, 0.1f, 0));
    double displayRenderTimeSeconds = 0;
    double displayDeltaTime = 0;
    int refreshRate = 500;
    bool wireframe = false;
    while (!glfwWindowShouldClose(window))
    {
        frameBeginTimeSeconds = getTimeSeconds();

        //render here
        GLCALL(glClear(GL_COLOR_BUFFER_BIT));
        if(wireframe) {
            GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        } else {
            GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }
        GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        renderTimeSeconds = getTimeSeconds() - frameBeginTimeSeconds;

        //dear imgui here
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("debug");
        ImGui::Text("FPS: %f", FPS);
        ImGui::Text("delta time: %fms", displayDeltaTime * 1000);
        ImGui::Text("render time: %fms", displayRenderTimeSeconds * 1000);
        ImGui::InputInt("refresh rate", &refreshRate);
        ImGui::Checkbox("wireframe", &wireframe);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        deltaTime = getTimeSeconds() - frameBeginTimeSeconds;
        if(iteration++ % (refreshRate > 0 ? refreshRate : 1) == 0) {
            FPS = deltaTime > 0 ? 1 / deltaTime : 0;
            displayRenderTimeSeconds = renderTimeSeconds;
            displayDeltaTime = deltaTime;
            glfwSetWindowTitle(window, (std::to_string((int) FPS) + " FPS").c_str());
        }
    }
}
