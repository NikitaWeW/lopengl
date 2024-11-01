#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "logger.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


#include "Application.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/Texture.hpp"
#include "opengl/Shader.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/Renderer.hpp"
#include "ControllableCamera.hpp"

#include <chrono>
#include <memory>
#include <thread>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

void imguistuff(double deltatime, double renderdeltatime, glm::vec3 &translation1, glm::vec3 &rotation1, glm::vec3 &scale1, ControllableCamera &cam, glm::vec3 &cuberotation)
{
    static bool wireframe = false;
    static glm::vec4 colorpicker(1);
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("aaa");
    ImGui::Separator();
    ImGui::Text("delta time: %f", deltatime);
    ImGui::Text("FPS: %f", deltatime ? 1 / deltatime : -1);
    ImGui::Separator();
    ImGui::Text("render delta time: %f", renderdeltatime);
    ImGui::Text("render FPS: %f", renderdeltatime ? 1 / renderdeltatime : -1);
    ImGui::Separator();
    if(ImGui::Checkbox("wireframe", &wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    ImGui::Separator();
    ImGui::Text("cube 1");
    ImGui::DragFloat3("position", &translation1.x, 0.01f);
    ImGui::DragFloat3("rotation", &rotation1.x, 0.5f);
    ImGui::DragFloat3("scale", &scale1.x, 0.01f);
    ImGui::InputFloat3("rotation per ms", &cuberotation.x);
    if (ImGui::Button("reset"))
    {
        translation1 = glm::vec3(0);
        rotation1 = glm::vec3(0);
        scale1 = glm::vec3(1);
    }
    ImGui::ColorEdit4("sample color picker", &colorpicker.x);
    ImGui::Separator();
    ImGui::Text("camera position: (%.3f; %.3f; %.3f)", cam.position.x, cam.position.y, cam.position.z);
    ImGui::Text("camera rotation: (%.3f; %.3f; %.3f)", cam.rotation.x, cam.rotation.y, cam.rotation.z);
    ImGui::InputFloat("camera speed", &cam.cameraSpeed);
    ImGui::InputFloat("camera sensitivity", &cam.sensitivity);
    if (ImGui::Button("reset camera"))
    {
        cam.position = {0, 0, 3};
        cam.rotation = {-90, 0, 0};
        cam.fov = 45;
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    ControllableCamera &camera = *static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        camera.mouseLocked = !camera.mouseLocked;
        if (camera.mouseLocked)
        {
            camera.firstCursorMove = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    float &fov = static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window))->fov;
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}
inline glm::vec3 toRGB(int hex)
{
    return {
        ((hex >> 16) & 0xFF) / 255.0,
        ((hex >> 8) & 0xFF) / 255.0,
        ((hex) & 0xFF) / 255.0};
}
int main()
{
float vertices[] = {
//      x      y      z        u      v      tex id
/*0*/  -0.5f, -0.5f, -0.5f,    0.0f,  0.0f,  1,  
/*1*/   0.5f, -0.5f, -0.5f,    2.0f,  0.0f,  1, 
/*2*/   0.5f,  0.5f, -0.5f,    0.0f,  2.0f,  1, 
/*3*/  -0.5f,  0.5f, -0.5f,    0.0f,  2.0f,  1, 
/*4*/  -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  0,
/*5*/   0.5f, -0.5f,  0.5f,    2.0f,  0.0f,  0,
/*6*/   0.5f,  0.5f,  0.5f,    2.0f,  2.0f,  0,
/*7*/  -0.5f,  0.5f,  0.5f,    0.0f,  2.0f,  0
};
    unsigned indices[] = {
        0, 1, 3, 3, 1, 2,
        1, 5, 2, 2, 5, 6,
        5, 4, 6, 6, 4, 7,
        4, 0, 7, 7, 0, 3,
        3, 2, 7, 7, 2, 6,
        4, 5, 0, 0, 5, 1
    };

    Application app;
    GLFWwindow *window = app.window;
    VertexBuffer VB(vertices, sizeof(vertices));
    IndexBuffer IB(indices, sizeof(indices));
    VertexArray VA;
    Texture tileTexture("res/textures/tile.png");
    Texture wallTexture("res/textures/wall.png");
    Shader shader("src/basic.glsl");
    VertexBufferlayout layout;
    ControllableCamera camera(window, {0, 0, 3}, {-90, 0, 0});
    double deltatime = 0;
    double renderdeltatime = 0;
    glm::vec3 cuberotation{0.1, 0.2, -0.1};

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, camera.mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    layout.push(3, GL_FLOAT);
    layout.push(2, GL_FLOAT);
    layout.push(1, GL_INT);

    tileTexture.bind(0);
    wallTexture.bind(1);
    VA.addBuffer(VB, layout);
    int samplers[] = { 0, 1 };
    shader.bind();
    glUniform1iv(shader.getUniform("u_Textures"), 2, samplers);

    glm::vec3 translation1 = glm::vec3(0.0f);
    glm::vec3 rotation1 = glm::vec3(0.0f);
    glm::vec3 scale1 = glm::vec3(1.0f);
    glm::vec3 ClearColor = toRGB(0x0f0f0d);

    std::thread cubeThread([&translation1, &rotation1, &scale1, window, &cuberotation](){
        while(!glfwWindowShouldClose(window)) {
            rotation1 += cuberotation;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfwGetWindowSize(window, &app.windowSize.x, &app.windowSize.y);

        glm::mat4 model(1.0f);
        model = glm::translate(model, translation1);
        model = glm::rotate(model, glm::radians(rotation1.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation1.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation1.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale1);

        glm::mat4 MVP = camera.getProjectionMatrix(app.windowSize.x, app.windowSize.y) * camera.getViewMatrix() * model;

        glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]);
        camera.update(deltatime);

        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.bind();
        IB.bind();
        VB.bind();
        glDrawElements(GL_TRIANGLES, IB.getSize(), GL_UNSIGNED_INT, nullptr);

        renderdeltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.000001;
        imguistuff(deltatime, renderdeltatime, translation1, rotation1, scale1, camera, cuberotation);

        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.000001;
    }
    cubeThread.join();
}
