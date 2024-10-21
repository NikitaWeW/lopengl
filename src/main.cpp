#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "Application.hpp"
#include "opengl/Renderer.hpp"
#include "opengl/GlCall.h"
#include "opengl/Shader.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/Texture.hpp"
#include "Camera.hpp"

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
    Camera cam(glm::vec3(0, 0, 2.5), glm::vec3(-90, 0, 0));
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

    glm::vec3 translation1(0);
    glm::vec3 rotation1(0);
    glm::vec3 scale1(1);
    glm::vec3 translation2(0);
    glm::vec3 rotation2(0);
    glm::vec3 scale2(1);
    bool wireframe = false;
    unsigned color1UniformLocation = shader.getUniform("u_color");
    float color[4] = {0, 0, 0, 1};
    bool showDemoWindow = false;
    const char* items[] = { "brick wall", "no texture"};
    int current_item = 0; // Index to store the selected item
    ImGuiIO &io = ImGui::GetIO();
    bool show_another_window = false;
    bool object2 = false;

    while (!glfwWindowShouldClose(window))
    {
        cam.update();
        glm::mat4 proj = cam.getProjectionMatrix(windowWidth, windowHeight);
        glm::mat4 view = cam.getViewMatrix();

        renderer.Clear(0.05, 0.1, 0.12);
        GLCALL(glUniform4f(color1UniformLocation, color[0], color[1], color[2], color[3]));

        { // object 1
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, translation1);
            model = glm::rotate(model, glm::radians(rotation1.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(rotation1.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(rotation1.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, scale1);
            glm::mat4  MVP = proj * view * model;
            GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));
            renderer.Draw(VA, IB, shader);
        }
        if(object2) { // object 2
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, translation2);
            model = glm::rotate(model, glm::radians(rotation2.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(rotation2.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(rotation2.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, scale2);
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
            ImGui::Checkbox("object 2", &object2);
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

            ImGui::Begin("square 1");
            ImGui::DragFloat3("position", &translation1.x, 0.01f);
            ImGui::DragFloat3("rotation", &rotation1.x, 0.5f);
            ImGui::DragFloat3("scale", &scale1.x, 0.01f);
            if(ImGui::Button("reset")) {
                translation1 = glm::vec3(0);
                rotation1 = glm::vec3(0);
                scale1 = glm::vec3(1);
            }
            ImGui::End();

            if(object2) {
                ImGui::Begin("square 2");
                ImGui::DragFloat3("position", &translation2.x, 0.01f);
                ImGui::DragFloat3("rotation", &rotation2.x, 0.5f);
                ImGui::DragFloat3("scale", &scale2.x, 0.01f);
                if(ImGui::Button("reset")) {
                    translation2 = glm::vec3(0);
                    rotation2 = glm::vec3(0);
                    scale2 = glm::vec3(1);
                }
                ImGui::End();
            }

            ImGui::Begin("view");
            ImGui::DragFloat3("position", &cam.position.x, 0.01f);
            ImGui::DragFloat3("rotation", &cam.rotation.x, 0.5f);
            if(ImGui::Button("reset")) {
                cam.position = glm::vec3(0, 0, 2.5);
                cam.rotation = glm::vec3(0);
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
