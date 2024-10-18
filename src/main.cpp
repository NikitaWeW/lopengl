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
    //  100.0f,  100.0f,  0.0f,    0.0f, 0.0f, // 0
    //  200.0f,  100.0f,  0.0f,    1.0f, 0.0f, // 1
    //  200.0f,  200.0f,  0.0f,    1.0f, 1.0f, // 2
    //  100.0f,  200.0f,  0.0f,    0.0f, 1.0f  // 3
    -0.5f, -0.5f,  0.0f,    0.0f, 0.0f, // 0
     0.5f, -0.5f,  0.0f,    1.0f, 0.0f, // 1
     0.5f,  0.5f,  0.0f,    1.0f, 1.0f, // 2
    -0.5f,  0.5f,  0.0f,    0.0f, 1.0f  // 3

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
    Texture smileTexture("res/textures/smile.png");

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
    glm::mat4 view(1.0f);
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(0.75f, 0, 0));
    // model = glm::rotate(model, 1.0f, glm::vec3(0, 0, 45));

    view = glm::translate(view, glm::vec3(-1, 0, 0));
    // view = glm::rotate(view, 1.0f, glm::vec3(0, 0, 45));

    glm::mat4 MVP = proj * view * model;
    GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));

    bool wireframe = false;
    unsigned color1UniformLocation = shader.getUniform("u_color");
    float color[4] = {0, 0, 0, 1};
    bool showDemoWindow = false;
    const char* items[] = { "brick wall", "smile", "no texture"};
    int current_item = 0; // Index to store the selected item
    ImGuiIO &io = ImGui::GetIO();
    bool show_another_window = false;

    while (!glfwWindowShouldClose(window))
    {
        GLCALL(glUniform4f(color1UniformLocation, color[0], color[1], color[2], color[3]));

        renderer.Clear(0.05, 0.1, 0.12);
        renderer.Draw(VA, IB, shader);

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
                case 1:
                    smileTexture.bind();
                    break;
                default:
                    brickWallTexture.unbind();
                    break;
                }
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
    }
}
