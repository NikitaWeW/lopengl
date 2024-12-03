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
#include "utils/ControllableCamera.hpp"
#include "utils/Model.hpp"
#include "utils/Light.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <stdexcept>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif
#define SHOW_LOGS true
#define LOAD_NOW true

void imguistuff(Application &app, ControllableCamera &cam, PointLight &light, SpotLight &flashlight);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main()
{
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f, 0.0f, 0.0f),
        glm::vec3( 2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f, 2.0f, -2.5f),
        glm::vec3( 1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };
    printf("loading...\n"); // TODO: cool console progress bar
    Application app;
    GLFWwindow *window = app.window;
    Shader lightCubeShader{"shaders/lightcube.glsl", SHOW_LOGS};
    Model lightCube("res/models/cube.obj");
    PointLight light;
        light.position= glm::vec3{2, 1, 3};
    ControllableCamera camera(window, {0, 0, 5}, {-90, 0, 0});
    SpotLight flashlight;
        flashlight.position= camera.position;
        flashlight.direction=camera.getFront();
    VertexBufferLayout layout;
    app.shaders = {
        Shader{"shaders/lighting.glsl", SHOW_LOGS},
        Shader{"shaders/basic.glsl",    SHOW_LOGS},
        Shader{"shaders/test.glsl",     SHOW_LOGS}
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. shows in shader list.
    Renderer renderer;
    renderer.getLights().push_back(&flashlight);
    renderer.getLights().push_back(&light);

//   ==========================================================
    app.addModel("res/models/cube.obj",               LOAD_NOW);
    app.addModel("res/models/backpack/backpack.obj", !LOAD_NOW);
//   ==========================================================
    app.addTexture("res/textures/tile.png",           LOAD_NOW);
    app.addTexture("res/textures/white.png",          LOAD_NOW);
    app.addTexture("res/textures/oak.jpg",           !LOAD_NOW);
    app.addTexture("res/textures/concrete.jpg",      !LOAD_NOW);
    app.addTexture("res/textures/brick_wall.jpg",    !LOAD_NOW);
//   ==========================================================

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

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

        glClearColor(app.clearColor.x, app.clearColor.y, app.clearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(unsigned i = 0; i < sizeof(cubePositions) / sizeof(*cubePositions); i++) {
            app.currentModel->resetMatrix();    
            app.currentModel->translate(cubePositions[i] * 2.0f);
            app.currentModel->rotate({20.0f * i, 13.0f * i, 1.5f * i});
            if(app.currentTexture) app.currentTexture->bind();
            renderer.draw(*app.currentModel, app.shaders[app.currentShaderIndex], camera);
            if(app.currentTexture) app.currentTexture->unbind();
        }
        
        lightCube.resetMatrix();
        lightCube.translate(light.position);
        lightCube.scale(glm::vec3{0.0625});

        renderer.draw(lightCube, lightCubeShader, camera);

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