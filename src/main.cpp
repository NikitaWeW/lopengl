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

void imguistuff(Application &app, ControllableCamera &cam, std::vector<Shader *> shaders, Light &light);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
float easeInOutSine(float x) {
    return -(cos(3.14 * x) - 1) / 2;
}

glm::vec3 interpolate(float n, glm::vec3 begin, glm::vec3 end, float (*easingfun)(float)) {
    return begin + easingfun(n) * (end - begin);
}

int main()
{
    printf("loading...\n"); // TODO: cool console progress bar
    Application app;
    GLFWwindow *window = app.window;
    Shader lightingShader("shaders/lighting.glsl");
    Shader lightCubeShader("shaders/lightcube.glsl");
    Model lightCube("res/models/cube.obj");
    Light light {
        .position= glm::vec3{2, 1, 3},
        .ambient = glm::vec3{0.1f},
        .diffuse = glm::vec3{0.5f},
        .specular= glm::vec3{1.0f}
    };
    VertexBufferLayout layout;
    ControllableCamera camera(window, {0, 0, 5}, {-90, 0, 0});
    std::vector<Shader *> shaders;
    shaders.push_back(&lightingShader);
    shaders.push_back(&lightCubeShader);

    app.addModel("res/models/Crate/Crate1.3ds");
    app.addModel("res/models/backpack/backpack.obj");
    app.addModel("res/models/cube.obj");
    app.addTexture("res/textures/tile.png");
    app.addTexture("res/textures/white.png");
    app.currentModel = &app.models[1];
    app.cuberotation = glm::vec3{0};

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
            app.currentModel->m_rotation += app.cuberotation;
            if(app.moveLight) {
                light.position = interpolate(app.updateCounter * app.lightSpeed, app.lightPosBegin, app.lightPosEnd, easeInOutSine);
            }
            camera.update(0.1);
            ++app.updateCounter;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfwGetWindowSize(window, &app.windowSize.x, &app.windowSize.y);

        glClearColor(app.clearColor.x, app.clearColor.y, app.clearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        if(app.currentModel) {
            app.currentModel->resetMatrix();
            app.currentModel->translate(app.currentModel->m_position);
            app.currentModel->rotate(app.currentModel->m_rotation);
            app.currentModel->scale(app.currentModel->m_scale);

            lightingShader.bind();
            if(lightingShader.getUniform("u_light.position") != -1) glUniform3fv(lightingShader.getUniform("u_light.position"), 1, &light.position.x);
            if(lightingShader.getUniform("u_light.ambient") != -1)  glUniform3fv(lightingShader.getUniform("u_light.ambient"),  1, &light.ambient.r);
            if(lightingShader.getUniform("u_light.diffuse") != -1)  glUniform3fv(lightingShader.getUniform("u_light.diffuse"),  1, &light.diffuse.r);
            if(lightingShader.getUniform("u_light.specular") != -1) glUniform3fv(lightingShader.getUniform("u_light.specular"), 1, &light.specular.r);
            glUniform3fv(lightingShader.getUniform("u_viewPos"), 1, &camera.position.x);

            if(app.currentTexture) app.currentTexture->bind();
            app.currentModel->draw(lightingShader, camera, app.windowSize.x, app.windowSize.y); 
            if(app.currentTexture) app.currentTexture->unbind();
        }
        
        lightCube.resetMatrix();
        lightCube.translate(light.position);
        lightCube.scale(glm::vec3{0.0625});

        lightCubeShader.bind();
        lightCube.draw(lightCubeShader, camera, app.windowSize.x, app.windowSize.y);
        lightCubeShader.unbind();

        imguistuff(app, camera, shaders, light);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
    updateThread.join();
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