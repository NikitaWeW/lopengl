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

glm::vec3 translation1 = glm::vec3{0.0f};
glm::vec3 rotation1 = glm::vec3{0.0f};
glm::vec3 scale1 = glm::vec3{1.0f};
glm::vec3 ClearColor = glm::vec3{0.0f};
float magicValue = 0;
float scrollSpeed  = 4.5f;
float updateCounter = 0;
float frequencyHZ = 0.1f;
double deltatime = 0;
double renderdeltatime = 0;
unsigned frameCounter = 0;
glm::vec3 cuberotation{0.1, 0.2, -0.1};
glm::vec3 lightColor{1};
glm::vec3 lightPos{2, 1, 3};

Model *currentModel = nullptr;
int currentModelIndex = 0;
std::vector<Model> models;
std::vector<std::string> modelNames;
char loadModelBuffer[1024];

Texture *currentTexture = nullptr;
int currentTextureIndex = 0;
std::vector<Texture> textures;
std::vector<std::string> textureNames;
char loadTextureBuffer[1024];

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
void addTexture(char const *filepath) {
    try {
        LOG_INFO("loading texture \"%s\"...", filepath);
        Texture texture{filepath};
        textures.push_back(texture);
        currentTexture = &textures[0];
        textureNames.push_back({filepath});
        LOG_INFO("texture loaded!");
    } catch(std::runtime_error &e) {
        LOG_ERROR("%s", e.what());
    }
}
void imguistuff(ControllableCamera &cam, std::vector<Shader *> shaders)
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
    ImGui::Text("render delta time: %f", renderdeltatime);
    ImGui::Text("render FPS: %f", renderdeltatime ? 1 / renderdeltatime : -1);
    ImGui::Separator();
    if(ImGui::Button("recompile shaders")) {
        for(Shader *shader : shaders) {
            Shader copy = *shader;
            LOG_INFO("recompiling %s...", shader->getFilePath().c_str());
            if(!shader->ParceShaderFile(shader->getFilePath())) {
                shader->swap(std::forward<Shader>(copy));
                continue;
            };
            if(!shader->CompileShaders()) {
                shader->swap(std::forward<Shader>(copy));
                continue;
            }
        }
        LOG_INFO("done.");
    }
    ImGui::Separator();
    if(ImGui::Checkbox("wireframe", &wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    ImGui::ColorEdit4("clear color", &ClearColor.x);
    // ImGui::InputFloat("dynamic geometry wobbly frequency™", &frequencyHZ);
    ImGui::Separator();
    ImGui::ColorEdit3("light color", &lightColor.r);
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
    ImGui::InputText("model path", loadModelBuffer, sizeof(loadModelBuffer));
    if(ImGui::Button("load model")) {
        addModel(loadModelBuffer);
    }
    ImGui::Separator();
    if(modelNames.size() > 0) {
        std::vector<const char *> textureCNames;
        for(std::string const &name : textureNames) textureCNames.push_back(name.c_str());
        if(ImGui::ListBox("loaded textures", &currentTextureIndex, textureCNames.data(), textureCNames.size())) {
            currentTexture = &textures[currentTextureIndex];
        }
    }
    ImGui::InputText("texture path", loadTextureBuffer, sizeof(loadTextureBuffer));
    if(ImGui::Button("load texture")) {
        addTexture(loadTextureBuffer);
    }
    ImGui::Separator();
    ImGui::Text("model 1");
    ImGui::DragFloat3("position", &translation1.x, 0.01f);
    ImGui::DragFloat3("rotation", &rotation1.x, 0.5f);
    ImGui::DragFloat3("scale", &scale1.x, 0.01f);
    ImGui::InputFloat3("rotation per ms", &cuberotation.x);
    if (ImGui::Button("reset model"))
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
        cam.position = {0, 0, 5};
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
    } else {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
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
    } else {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    }
}

int main()
{
    printf("loading...\n"); // TODO: cool console progress bar
    Application app;
    GLFWwindow *window = app.window;
    Shader lightingShader("shaders/lighting.glsl");
    Shader lightCubeShader("shaders/lightcube.glsl");
    Model lightCube("res/models/cube.obj");
    VertexBufferLayout layout;
    ControllableCamera camera(window, {0, 0, 5}, {-90, 0, 0});
    std::vector<Shader *> shaders;
    shaders.push_back(&lightingShader);
    shaders.push_back(&lightCubeShader);

    strcpy(loadModelBuffer, "");
    addModel("res/models/Crate/Crate1.3ds");
    addModel("res/models/backpack/backpack.obj");
    addModel("res/models/cube.obj");
    addTexture("res/textures/tile.png");
    addTexture("res/textures/white.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, camera.mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    printf("loaded!\n");

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

        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelMatrix;

        if(currentModel) {
            modelMatrix = glm::translate(glm::mat4{1.0f}, translation1);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation1.x), glm::vec3(1, 0, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation1.y), glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation1.z), glm::vec3(0, 0, 1));
            modelMatrix = glm::scale(modelMatrix, scale1);

            lightingShader.bind();
            glUniform3fv(lightingShader.getUniform("u_lightColor"), 1, &lightColor.r);
            glUniform3fv(lightingShader.getUniform("u_lightPos"), 1, &lightPos.x);

            if(currentTexture) currentTexture->bind();
            currentModel->draw(lightingShader, modelMatrix, camera, app.windowSize.x, app.windowSize.y); 
            if(currentTexture) currentTexture->unbind();
        }
        
        modelMatrix = glm::translate(glm::mat4{1.0f}, lightPos);
        modelMatrix = glm::scale(modelMatrix, glm::vec3{0.125});

        lightCubeShader.bind();
        lightCube.draw(lightCubeShader, modelMatrix, camera, app.windowSize.x, app.windowSize.y);
        lightCubeShader.unbind();

        renderdeltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
        imguistuff(camera, shaders);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++frameCounter;
        deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
    updateThread.join();
}
