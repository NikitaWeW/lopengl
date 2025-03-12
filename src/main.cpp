/*
i use this (gcc + ninja)
cmake -S . -B build -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS='-fdiagnostics-color=always -Wall' -G Ninja
cmake --build build && build/main

        +____________+
        /:\         ,:\
       / : \       , : \
      /  :  \     ,  :  \
     /   :   +-----------+
    +....:../:...+   :  /|
    |\   +./.:...`...+ / |
    | \ ,`/  :   :` ,`/  |
    |  \ /`. :   : ` /`  |
    | , +-----------+  ` |
    |,  |   `+...:,.|...`+
    +...|...,'...+  |   /
     \  |  ,     `  |  /
      \ | ,       ` | /
       \|,         `|/
        +___________+

2-Dimensional Representation Of A 3-Dimensional Cross-Section Of A 4-Dimensional Cube
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
#include "random.hpp"
#include "opengl/Renderer.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Framebuffer.hpp"
#include "opengl/UniformBuffer.hpp"
#include "opengl/Cubemap.hpp"
#include "opengl/ShaderStorageBuffer.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
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

void imguistuff(Application &app, ControllableCamera &cam);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main(int argc, char **argv)
{
    printf("loading...\n"); // TODO: cool progress bar
    Application app;
    GLFWwindow *window = app.window;
    app.camera = ControllableCamera(window, {0, 0, 3}, {-90, 0, 0});
    app.camera.far = 500;
    Renderer renderer;
    
    app.shaders = {
        {"shaders/lighting.glsl",              SHOW_LOGS}, // 0
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 

    Model cube{"res/models/cube.obj", !FLIP_TEXTURES, FLIP_WINING_ORDER};
    Model quad{"res/models/quad.obj"};

    cube.getMeshes()[0].textures.push_back(Texture{"res/textures/oak.jpg", false, true, GL_CLAMP_TO_EDGE, GL_LINEAR, "diffuse"});

//  ----------------------------------
    // input here
    constexpr unsigned numLights = 50;
    constexpr unsigned numModels = 1000;
    constexpr float radius = 50;
    Model sceneModel = cube;
//  ----------------------------------

    glm::mat4 modelMatrices[numModels];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    for(unsigned i = 0; i < numModels; ++i) {
        glm::mat4 model{1.0f};
        model = glm::translate(model, { randRange(-radius, radius), randRange(-radius, radius), randRange(-radius, radius) });
        model = glm::scale(model, glm::vec3{ randRange(0.5f, 1.5f) });
        model = glm::rotate(model, 1.0f, {randRange(0.0f, 360.0f), randRange(0.0f, 360.0f), randRange(0.0f, 360.0f)});
        modelMatrices[i] = model;
    }

    SSBO matricesSSBO{numModels * sizeof(glm::mat4)};
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numModels * sizeof(glm::mat4), modelMatrices);

    PointLight lights[numLights];
    for(unsigned i = 0; i < numLights; ++i) {
        PointLight light;
        light.position = { randRange(-radius, radius), randRange(-radius, radius), randRange(-radius, radius) };
        light.color = { randRange(0.0f, 2.0f), randRange(0.0f, 2.0f), randRange(0.0f, 2.0f) };
        light.attenuation = randRange(0.07f, 0.09f);
        lights[i] = light; // keep them in the memory
        renderer.getLights().push_back(lights + i);
    }

// =========================== //


    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        app.camera.update(app.deltatime);
        glfwGetWindowSize(window, &app.camera.width, &app.camera.height);

        glViewport(0, 0, app.camera.width, app.camera.height);
        renderer.clear(app.clearColor);

        app.shaders[0].bind();
        matricesSSBO.bind(0);
        glShaderStorageBlockBinding(app.shaders[0].getRenderID(), app.shaders[0].getStorageBlock("modelSSBO"), 0);
        
        // set uniforms
        glUniformMatrix4fv(app.shaders[0].getUniform("u_viewMat"),      1, GL_FALSE, &app.camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_projectionMat"),1, GL_FALSE, &app.camera.getProjectionMatrix()[0][0]);
        glUniform3fv(app.shaders[0].getUniform("u_viewPos"), 1, &app.camera.position.x);
        renderer.setLightingUniforms(app.shaders[0]);

        for(Mesh const &mesh : sceneModel.getMeshes()) {
            bool specularSet = false;
            bool normalSet = false;
            bool heightSet = false;
            unsigned int textureCount = 0;
            for(Texture const &texture : mesh.textures) {
                int location = app.shaders[0].getUniform("u_material." + texture.type);
                if(location != -1) {
                    glUniform1i(location, textureCount);
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

            glUniform1i(app.shaders[0].getUniform("u_material.specularSet"), specularSet);
            glUniform1i(app.shaders[0].getUniform("u_material.normalSet"), normalSet);
            glUniform1i(app.shaders[0].getUniform("u_material.heightSet"), heightSet);
         
            
            mesh.va.bind();
            mesh.ib.bind();
            glDrawElementsInstanced(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr, numModels);
        }


        if(app.frameCounter % 100 == 0) glfwSetWindowTitle(window, ("lopengl -- " + std::to_string((int) glm::round(1 / app.deltatime)) + " FPS").c_str());
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