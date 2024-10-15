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
#include "Shader.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"

#include <iostream> //std includes
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
    -0.5f, -0.5f,  0.0f,    0.0f, 0.0f, // 0
     0.5f, -0.5f,  0.0f,    1.0f, 0.0f, // 1
     0.5f,  0.5f,  0.0f,    1.0f, 1.0f, // 2
    -0.5f,  0.5f,  0.0f,    0.0f, 1.0f  // 3
};
unsigned const indicies[] = {
    0, 1, 2,
    0, 2, 3
};


double getTimeSeconds() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count(); 
}
int main()
{
    Application app;
    Renderer renderer;
    Shader shaderProg;
    VertexBuffer VB(vertices, sizeof(vertices));
    IndexBuffer IB(indicies, 6);
    VertexArray VA;
    VertexBufferlayout layout;
    Texture brickWallTexture("res/textures/wall.png");
    Texture smileTexture("res/textures/smile.png");
    if(!shaderProg.ParceShaderFile("src/basic.glsl")) {
        LOG_FATAL("failed to parce shader.");
        return -1;
    }
    if(!shaderProg.CompileShaders()) {
        LOG_FATAL("failed to compile shaders.");
        return -1;
    }

    layout.push<float>(3);
    layout.push<float>(2);
    shaderProg.bind();
    GLCALL(glUniform1i(shaderProg.getUniform("u_Texture"), 0));
    brickWallTexture.bind();
    VA.bind();
    VA.addBuffer(VB, layout);
    
    double displayRenderTimeSeconds = 0;
    double displayDeltaTime = 0;
    int refreshRate = 500;
    bool wireframe = false;
    unsigned colorUniformLocation = shaderProg.getUniform("u_color");
    float color[3] = {1, 1, 1};
    bool showDemoWindow = false;
    
    while (!glfwWindowShouldClose(window))
    {
        frameBeginTimeSeconds = getTimeSeconds();

        renderer.Clear(0.05, 0.1, 0.12);

        //render here
        if(wireframe) {
            GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        } else {
            GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }
        GLCALL(glUniform4f(colorUniformLocation, color[0], color[1], color[2], 0));
        renderer.Draw(VA, IB, shaderProg);
        renderTimeSeconds = getTimeSeconds() - frameBeginTimeSeconds;

        //dear imgui here
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(showDemoWindow) ImGui::ShowDemoWindow();

        ImGui::Begin("debug");
        ImGui::Text("FPS: %f", FPS);
        ImGui::Text("renderFPS: %f", renderFPS);
        ImGui::Text("delta time: %fms", displayDeltaTime * 1000);
        ImGui::Text("render time: %fms", displayRenderTimeSeconds * 1000);
        ImGui::InputInt("refresh rate", &refreshRate);
        ImGui::End();

        ImGui::Begin("properties");
        ImGui::ColorEdit3("color", color);
        ImGui::Checkbox("wireframe", &wireframe);
        ImGui::Checkbox("demo window", &showDemoWindow);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(window);
        glfwPollEvents();

        deltaTime = getTimeSeconds() - frameBeginTimeSeconds;
        if(iteration++ % (refreshRate > 0 ? refreshRate : 1) == 0) {
            FPS = deltaTime > 0 ? 1 / deltaTime : 0;
            renderFPS = renderTimeSeconds > 0 ? 1 / renderTimeSeconds : 0;
            displayRenderTimeSeconds = renderTimeSeconds;
            displayDeltaTime = deltaTime;
            glfwSetWindowTitle(window, (std::to_string((int) FPS) + " FPS").c_str());
        }
    }
}
