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
#include "opengl/Renderer.hpp"
#include "utils/ControllableCamera.hpp"
#include "scene/scenes.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <stdexcept>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif
#define SHOW_LOGS true // for readability
#define LOAD_NOW true

void imguistuff(Application &app, ControllableCamera &cam, PointLight &light, SpotLight &flashlight);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main()
{
    printf("loading...\n"); // TODO: cool console progress bar
    Application app;
    GLFWwindow *window = app.window;
    Model lightCube("res/models/cube.obj");
    ControllableCamera camera(window, {0, 0, 5}, {-90, 0, 0});
    PointLight light;
    SpotLight flashlight;
    VertexBufferLayout layout;
    Renderer renderer;
    scenes::CubePartyScene cubeParty;
    scenes::SingleModelScene singleModel;

//  =========================================== 

    renderer.getLights().push_back(&flashlight);
    renderer.getLights().push_back(&light);

    app.shaders = {
        {"shaders/lighting.glsl", SHOW_LOGS},
        {"shaders/basic.glsl",    SHOW_LOGS},
        {"shaders/test.glsl",     SHOW_LOGS}
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. shows in shader list.
    app.scenes = {
        &cubeParty,
        &singleModel
    };

    flashlight.position  = camera.position;
    flashlight.direction = camera.getFront();
    flashlight.enabled   = false;

    light.position= glm::vec3{2, 1, 3};

    app.plainColorShader = Shader{"shaders/plain_color.glsl", SHOW_LOGS};

//   ==================================================================

    app.addModel("res/models/cube.obj",                             LOAD_NOW);
    app.addModel("res/models/backpack/backpack.obj",               !LOAD_NOW);
    app.addModel("res/models/rock/namaqualand_cliff_02_4k.gltf",   !LOAD_NOW, true);
    app.addModel("res/models/lemon/lemon_4k.gltf",                 !LOAD_NOW, true);
    app.addModel("res/models/apple/food_apple_01_4k.gltf",         !LOAD_NOW, true);
//   =======================================================================
    app.addTexture("res/textures/tile.png",           LOAD_NOW);
    app.addTexture("res/textures/white.png",          LOAD_NOW);
    app.addTexture("res/textures/oak.jpg",           !LOAD_NOW);
    app.addTexture("res/textures/concrete.jpg",      !LOAD_NOW);
    app.addTexture("res/textures/brick_wall.jpg",    !LOAD_NOW);

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, camera.mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    LOG_INFO("loaded!");

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        camera.update(app.deltatime);
        flashlight.position  = camera.position;
        flashlight.direction = camera.getFront();
        glfwGetWindowSize(window, &camera.windowWidthPx, &camera.windowHeightPx);
        
        app.scenes[app.currentSceneIndex]->onRender(app, renderer, camera);

        lightCube.resetMatrix();
        lightCube.translate(light.position);
        lightCube.scale(glm::vec3{0.03125});

        if(light.enabled) {
            app.plainColorShader.bind();
            glUniform3fv(app.plainColorShader.getUniform("u_color"), 1, &light.color.x);
            renderer.draw(lightCube, app.plainColorShader, camera);
        }

        imguistuff(app, camera, light, flashlight);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
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
        cam->fov -= (float)yoffset * 4.5f;
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