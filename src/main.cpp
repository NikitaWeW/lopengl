/*
i use this (gcc + ninja)
cmake -S . -B build -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS='-fdiagnostics-color=always -Wall' -G Ninja
cmake --build build && build/main
*/

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
#include "opengl/Framebuffer.hpp"
#include "opengl/UniformBuffer.hpp"
#include "opengl/Cubemap.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <stdexcept>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif
#define SHOW_LOGS         true // for readability
#define LOAD_NOW          true
#define FLIP_TEXTURES     true
#define FLIP_WINING_ORDER true
#define SRGB              true
#define currentShader app.shaders[app.displayShaders[app.currentShaderIndex]]

void imguistuff(Application &app, ControllableCamera &cam, PointLight &light, SpotLight &flashlight, DirectionalLight &sun);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main(int argc, char **argv)
{
    printf("loading...\n"); // TODO: cool progress bar
    Application app;
    GLFWwindow *window = app.window;
    ControllableCamera camera(window, {0, 0, -3}, {90, 0, 0});
    PointLight light;
    DirectionalLight sun;
    SpotLight flashlight;
    Renderer renderer;
    Cubemap skybox("res/textures/skybox1", {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg"});
    
//  =========================================== 

    glfwGetWindowSize(window, &camera.width, &camera.height);

    renderer.getLights().push_back(&flashlight);
    renderer.getLights().push_back(&light);
    renderer.getLights().push_back(&sun);

    app.shaders = {
        {"shaders/basic.glsl",          SHOW_LOGS}, // 0
        {"shaders/lighting.glsl",       SHOW_LOGS}, // 1
        {"shaders/reflection.glsl",     SHOW_LOGS}, // 2
        {"shaders/refraction.glsl",     SHOW_LOGS}, // 3
        {"shaders/explode.glsl",        SHOW_LOGS}, // 4
//       =========================================
        {"shaders/post_process.glsl",   SHOW_LOGS}, // 5
        {"shaders/depth_omnidir.glsl",  SHOW_LOGS}, // 6
        {"shaders/depth_regular.glsl",  SHOW_LOGS}, // 7
        {"shaders/plain_color.glsl",    SHOW_LOGS}, // 8
        {"shaders/skybox.glsl",         SHOW_LOGS}, // 9
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 
    app.displayShaders = {0, 1, 2, 3, 4}; // shows in shader list.

    flashlight.position  = camera.position;
    flashlight.direction = camera.getFront();
    light.position= glm::vec3{1, 1, -2};
    sun.direction = glm::vec3{1, -0.5f, 0.5f};

    flashlight.enabled = false;
    sun.enabled        = false;
    light.enabled      = true;

    app.cube = Model{"res/models/cube.obj", !FLIP_TEXTURES, FLIP_WINING_ORDER};
    app.camera = &camera;

//   ==================================================================
    app.models = {
        app.cube,
        {"res/models/sphere/scene.gltf",                   FLIP_TEXTURES },
        {"res/models/lemon/lemon_4k.gltf",                 FLIP_TEXTURES },
        {"res/models/wall/wall.obj",                       FLIP_TEXTURES },
        {"res/models/backpack/backpack.obj",              !FLIP_TEXTURES },
        {"res/models/sponza/sponza.obj",                   FLIP_TEXTURES },
    };
    { // do model specific stuff
        Texture heightMap{"res/models/wall/height.jpg", FLIP_TEXTURES, !SRGB, GL_REPEAT, GL_NEAREST};
        heightMap.type = "height";
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/wall/wall.obj"; })->getMeshes()[0].textures.push_back(heightMap);

        for(Mesh &mesh : std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/sponza/sponza.obj"; })->getMeshes()) {
            for(Texture &texture : mesh.textures) {
                if(texture.type == "normal") texture.type = "height";
            }
        }
    }

// =========================== //
    // how to get segfault 101
    app.currentModelIndex = 3;    // wall
    app.currentShaderIndex = 1;   // lighting

// =========================== //

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_CURSOR, camera.locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    LOG_INFO("loaded!");

// =========================== //

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        camera.update(app.deltatime);
        flashlight.position  = camera.position;
        flashlight.direction = camera.getFront();
        glfwGetWindowSize(window, &camera.width, &camera.height);

// ===================== //
//     draw the scene    //
// ===================== //

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear(app.clearColor);

        currentShader.bind();
        // set uniforms
        glUniform3fv(currentShader.getUniform("u_viewPos"), 1, &camera.position.x);
        glUniformMatrix4fv(currentShader.getUniform("u_viewMat"),      1, GL_FALSE, &camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
        renderer.setLightingUniforms(currentShader); // it sucks.

// ================== //

        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.currentModelPosition);
        app.models[app.currentModelIndex].rotate(app.currentModelRotation);
        app.models[app.currentModelIndex].scale(app.currentModelScale);
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &app.models[app.currentModelIndex].getModelMat()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(app.models[app.currentModelIndex].getModelMat()))[0][0]);
        for(Mesh const &mesh : app.models[app.currentModelIndex].getMeshes()) {
            bool specularSet = false;
            bool normalSet = false;
            bool heightSet = false;
            unsigned int textureCount = 1; // leave 0 for other purposes
            for(Texture const &texture : mesh.textures) {
                int location = currentShader.getUniform("u_material." + texture.type);
                if(location != -1) {
                    glUniform1i(location, textureCount); // 5 nests wtf
                    texture.bind(textureCount);
                    ++textureCount;
                }
                if(texture.type == "specular") {
                    specularSet = true;
                } else if(texture.type == "normal") {
                    normalSet = true;
                } else if(texture.type == "height") {
                    heightSet = true;
                }
            }
            glUniform1f(currentShader.getUniform("u_material.shininess"), mesh.material.shininess);

            glUniform1i(currentShader.getUniform("u_material.specularSet"), specularSet);
            glUniform1i(currentShader.getUniform("u_material.normalSet"), normalSet);
            glUniform1i(currentShader.getUniform("u_material.heightSet"), heightSet);
            
            renderer.draw(mesh);
        }

// ================== //
        
        if(light.enabled) {
            // draw the light cube
            app.cube.resetMatrix();
            app.cube.translate(light.position);
            app.cube.scale(glm::vec3{0.03125});
            app.shaders[8].bind();
            glUniform3fv(app.shaders[8].getUniform("u_color"), 1, &light.color.x);
            glUniformMatrix4fv(app.shaders[8].getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
            glUniformMatrix4fv(app.shaders[8].getUniform("u_viewMat"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
            glUniformMatrix4fv(app.shaders[8].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
            renderer.draw(app.cube);
        } 

// ================== //

        imguistuff(app, camera, light, flashlight, sun);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
}
/*
c++ be like:
Because the lvalueness or rvalueness of an expression is independent of its type, it’s possible to have lvalues whose type is rvalue reference, and it’s also possible to have rvalues of the type rvalue reference.
*/