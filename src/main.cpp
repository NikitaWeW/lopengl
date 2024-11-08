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
#include "opengl/Vertex.hpp"
#include "Model.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <stdexcept>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

inline glm::vec3 toRGB(int hex);
glm::vec3 translation1 = glm::vec3{0.0f};
glm::vec3 rotation1 = glm::vec3{0.0f};
glm::vec3 scale1 = glm::vec3{1.0f};
glm::vec3 ClearColor = toRGB(0x0f0f0d); // why not?
float magicValue = 0;
float scrollSpeed  = 4.5f;
float updateCounter = 0;
float frequencyHZ = 0.1f;
double deltatime = 0;
double renderdeltatime = 0;
unsigned frameCounter = 0;
glm::vec3 cuberotation{0.1, 0.2, -0.1};
glm::vec4 lightColor{1};
glm::vec3 lightPos{0};
int currentModelIndex = 0;
Model *currentModel = nullptr;
std::vector<Model> models;
std::vector<std::string> modelNames;
char loadModelBuffer[1024];

void addModel(char const *filepath) {
    try {
        LOG_INFO("loading model \"%s\"...", filepath);
        std::string newFilepath{filepath};
        std::replace_if(newFilepath.begin(), newFilepath.end(), [](char c){ return c == '\\'; }, '/');
        Model model{newFilepath};
        models.push_back(std::move(model));
        currentModel = nullptr;
        modelNames.push_back({filepath});
        LOG_INFO("model loaded!");
    } catch(std::runtime_error &e) {
        LOG_ERROR("%s", e.what());
    }
}
void imguistuff(ControllableCamera &cam, Texture &defaultTexture)
{
    static bool wireframe = false;
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("properties");
    ImGui::Text("<esc> -- (un)lock mouse + keyboard");
    ImGui::Text("W, A, S, D, E, Q -- move");
    ImGui::Text("<mouse> -- look around");
    ImGui::Text("<scroll> -- zoom");
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
    ImGui::ColorEdit4("clear color", &ClearColor.x);
    // ImGui::InputFloat("dynamic geometry wobbly frequency™", &frequencyHZ);
    ImGui::ColorEdit4("light color", &lightColor.r);
    ImGui::DragFloat3("light position", &lightPos.x, 0.01f);
    ImGui::Separator();
    if(currentModel) {
        size_t triangles = 0;
        for(size_t i = 0; i < currentModel->getMeshes().size(); ++i) {
            triangles += currentModel->getMeshes()[i].indices.size() / 3;
        }
        ImGui::Text("%u triangles", triangles);
    }
    if(modelNames.size() > 0) {
        std::vector<const char *> modelCNames;
        for(std::string &name : modelNames) modelCNames.push_back(name.c_str());
        if(ImGui::ListBox("loaded models", &currentModelIndex, modelCNames.data(), modelCNames.size())) {
             currentModel = &models.at(currentModelIndex);
        }
    }
    ImGui::InputText("load model", loadModelBuffer, sizeof(loadModelBuffer));
    if(ImGui::Button("load")) {
        addModel(loadModelBuffer);
    }
    if(ImGui::Button("apply default texture")) {
        defaultTexture.bind();
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
        cuberotation = glm::vec3(0);
        frequencyHZ = 0;
        magicValue = 0.5f;
    }
    ImGui::Separator();
    ImGui::DragFloat3("camera position", &cam.position.x, 0.01f);
    ImGui::DragFloat3("camera rotation", &cam.rotation.x, 0.5f);
    ImGui::DragFloat("camera near plane", &cam.near, 0.001f);
    ImGui::DragFloat("camera far plane", &cam.far, 25.0f);
    ImGui::InputFloat("camera speed", &cam.cameraSpeed);
    ImGui::InputFloat("camera scroll speed", &scrollSpeed);
    ImGui::InputFloat("camera sensitivity", &cam.sensitivity);
    if (ImGui::Button("reset camera"))
    {
        cam.position = {0, 0, 3};
        cam.rotation = {-90, 0, 0};
        cam.fov = 45;
        cam.near = 0.01f;
        cam.far = 1000.0f;
        cam.cameraSpeed = 1.0f;
        cam.sensitivity = 1.0f;
        scrollSpeed = 4.5f;
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
    if(key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
        // evaluate fov
        if (camera.fov < 1.0f)
            camera.fov = 1.0f;
        if (camera.fov > 45.0f)
            camera.fov = 45.0f;
    }
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    ControllableCamera *cam = static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if(cam->mouseLocked) {
        cam->fov -= (float)yoffset * scrollSpeed;
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
    }
}
glm::vec3 toRGB(int hex)
{
    return {
        ((hex >> 16) & 0xFF) / 255.0,
        ((hex >> 8) & 0xFF) / 255.0,
        ((hex) & 0xFF) / 255.0};
}

int main()
{
    printf("loading...\n"); // TODO: console progress bar
    Application app;
    GLFWwindow *window = app.window;
    Shader shader("src/basic.glsl");
    Texture texture("res/textures/tile.png");
    VertexBufferLayout layout;
    ControllableCamera camera(window, {0, 0, 3}, {-90, 0, 0});

    strcpy(loadModelBuffer, "");
    addModel("res/models/Crate/Crate1.3ds");
    addModel("res/models/backpack/backpack.obj");


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, camera.mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    printf("loaded!\n");

    shader.bind();
    texture.bind();

    std::thread updateThread([&, window]() {
        while(!glfwWindowShouldClose(window)) {
            rotation1 += cuberotation;
            magicValue = sin(0.0062831852f * updateCounter * frequencyHZ); // magic happens
            camera.update(0.1);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            ++updateCounter;
        }
    });

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfwGetWindowSize(window, &app.windowSize.x, &app.windowSize.y);

        glm::mat4 modelmat(1.0f);
        modelmat = glm::translate(modelmat, translation1);
        modelmat = glm::rotate(modelmat, glm::radians(rotation1.x), glm::vec3(1, 0, 0));
        modelmat = glm::rotate(modelmat, glm::radians(rotation1.y), glm::vec3(0, 1, 0));
        modelmat = glm::rotate(modelmat, glm::radians(rotation1.z), glm::vec3(0, 0, 1));
        modelmat = glm::scale(modelmat, scale1);

        glm::mat4 MVP = camera.getProjectionMatrix(app.windowSize.x, app.windowSize.y) * camera.getViewMatrix() * modelmat;

        glUniform4fv(shader.getUniform("u_lightColor"), 1, &lightColor.r);
        glUniform3fv(shader.getUniform("u_lightPos"), 1, &lightPos.x);
        glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]);

        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if(currentModel) currentModel->draw(shader); 

        renderdeltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
        imguistuff(camera, texture);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++frameCounter;
        deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
    updateThread.join();
}
