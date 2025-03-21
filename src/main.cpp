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
float lerp(float a, float b, float x) { return a + x * (b - a); }

int main(int argc, char **argv)
{
    Application app; // initialisation
    app.camera = ControllableCamera{app.window, {0, 0, 4}, {-90, 0, 0}};
    app.shaders = {
        {"shaders/basic.glsl",              SHOW_LOGS}, // 0
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 

    Model cube{"res/models/cube.obj", !FLIP_TEXTURES, FLIP_WINING_ORDER};
    Model sphere{"res/models/sphere.obj", FLIP_TEXTURES };

    // glm::vec4 *data = new glm::vec4[count];
    // for(unsigned i = 0; i < count; ++i) {
    //     data[i] = {randRange(0.0f, 1.0f), randRange(0.0f, 1.0f), randRange(0.0f, 1.0f), 1};
    // }
    unsigned count = 3;
    float data[] = {
        0, 1, 0, 1,
        1, 1, 0, 1, 
        1, 1, 1, 1
    };
    UniformBuffer ubo{sizeof(GLfloat) + sizeof(glm::vec4) * 200 };
    ubo.bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4) * count, data);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 200, sizeof(GLfloat), &count);
    ubo.bindingPoint(0);

    while (!glfwWindowShouldClose(app.window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        app.camera.update(app.deltatime);
        glfwGetWindowSize(app.window, &app.camera.width, &app.camera.height);
// ================== //
//  geometry pass
// ================== //

        glViewport(0, 0, app.camera.width, app.camera.height);
        glClearColor(app.clearColor.r, app.clearColor.g, app.clearColor.b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        app.shaders[0].bind();
        glUniformBlockBinding(app.shaders[0].getRenderID(), app.shaders[0].getUniformBlock("vectors"), 0);
        cube.resetMatrix();
        glUniformMatrix4fv(app.shaders[0].getUniform("u_viewMat"),      1, GL_FALSE, &app.camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_projectionMat"),1, GL_FALSE, &app.camera.getProjectionMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_modelMat"), 1, GL_FALSE, &cube.getModelMat()[0][0]);
        for(Mesh const &mesh : cube.getMeshes()) {
            mesh.va.bind();
            mesh.ib.bind();
            glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        }
        
        if(app.frameCounter % 100 == 0) glfwSetWindowTitle(app.window, ("lopengl -- " + std::to_string((int) glm::round(1 / app.deltatime)) + " FPS").c_str());
        glfwSwapBuffers(app.window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
}
/*
c++ be like:
Because the lvalueness or rvalueness of an expression is independent of its type, it’s possible to have lvalues whose type is rvalue reference, and it’s also possible to have rvalues of the type rvalue reference.
*/