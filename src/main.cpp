#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include "Application.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

float const vertices[] = {
    -1.0f, -1.0f,  0.0f,    0.0f, 0.0f, // 0
     1.0f, -1.0f,  0.0f,    1.0f, 0.0f, // 1
     1.0f,  1.0f,  0.0f,    1.0f, 1.0f, // 2
    -1.0f,  1.0f,  0.0f,    0.0f, 1.0f  // 3

};
unsigned const indicies[] = {
    0, 1, 2,
    0, 2, 3
};

int main()
{
    Application app;
    GLFWwindow *window = app.window;
    Renderer renderer;
    Shader shader;
    VertexBuffer VB(vertices, sizeof(vertices));
    IndexBuffer IB(indicies, 6);
    VertexArray VA;
    VertexBufferlayout layout;
    Texture brickWallTexture("res/textures/wall.png");

    int windowWidth;
    int windowHeight;
    // float aspect = (float) windowWidth / windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    if(!shader.ParceShaderFile("src/basic.glsl")) return -1;
    if(!shader.CompileShaders()) return -1;

    layout.push<float>(3);
    layout.push<float>(2);
    shader.bind();
    brickWallTexture.bind();
    GLCALL(glUniform1i(shader.getUniform("u_Texture"), 0));
    VA.bind();  
    VA.addBuffer(VB, layout);

    glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
    // glm::mat4 proj = glm::perspective(1.0f, (float) windowWidth / windowHeight, -1.0f, 100.0f);
    glm::mat4 view(1.0f);

    glm::vec3 translation(0);
    glm::vec3 rotation(0);
    glm::vec3 scale(1);
    glm::vec3 viewTranslation(0);
    glm::vec3 viewRotation(0);
    bool wireframe = false;
    unsigned color1UniformLocation = shader.getUniform("u_color");
    float color[4] = {0, 0, 0, 1};
    bool showDemoWindow = false;
    const char* items[] = { "brick wall", "no texture"};
    int current_item = 0; // Index to store the selected item
    ImGuiIO &io = ImGui::GetIO();
    bool show_another_window = false;

    while (!glfwWindowShouldClose(window))
    {
        view = glm::mat4(1.0f);
        view = glm::translate(view, -viewTranslation);

        renderer.Clear(0.05, 0.1, 0.12);
        GLCALL(glUniform4f(color1UniformLocation, color[0], color[1], color[2], color[3]));

        { // object 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, translation);
            model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
            model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
            model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
            model = glm::scale(model, scale);
            glm::mat4  MVP = proj * view * model;
            GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));
            renderer.Draw(VA, IB, shader);
        }
        { // object 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(1.5, 0, 0));
            model = glm::rotate(model, 0.0f, glm::vec3(1, 0, 0));
            model = glm::rotate(model, 0.0f, glm::vec3(0, 1, 0));
            model = glm::rotate(model, 0.0f, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(1, 1, 1));
            glm::mat4  MVP = proj * view * model;
            GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));
            renderer.Draw(VA, IB, shader);
        }

        { //imgui goes here
            if(wireframe) {
                GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            } else {
                GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("properties");
            ImGui::ColorEdit4("color", color);
            ImGui::Checkbox("wireframe", &wireframe);
            if(ImGui::Combo("texture", &current_item, items, IM_ARRAYSIZE(items))) {
                switch (current_item) {
                case 0:
                    brickWallTexture.bind();
                    break;
                default:
                    brickWallTexture.unbind();
                    break;
                }
            }
            ImGui::End();

            ImGui::Begin("square");
            ImGui::DragFloat3("translation", &translation.x, 0.01f);
            ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
            ImGui::DragFloat3("scale", &scale.x, 0.01f);
            if(ImGui::Button("reset")) {
                translation = glm::vec3(0);
                rotation = glm::vec3(0);
                scale = glm::vec3(1);
            }
            ImGui::End();

            ImGui::Begin("view");
            ImGui::DragFloat3("translation", &viewTranslation.x, 0.01f);
            ImGui::DragFloat3("rotation", &viewRotation.x, 0.01f);
            if(ImGui::Button("reset")) {
                viewTranslation = glm::vec3(0);
                viewRotation = glm::vec3(0);
            }
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
        }
    
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
    }
}
