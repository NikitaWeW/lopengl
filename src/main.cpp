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

void imguistuff(Application &app);
float lerp(float a, float b, float x) { return a + x * (b - a); }

int main(int argc, char **argv)
{
    Application app; // initialisation
    app.camera = ControllableCamera{app.window, {0, 0, 0}, {-90, 0, 0}};
    app.shaders = {
        {"shaders/raytracing.glsl", true},
        {"shaders/hdr.glsl",        true}
    };

    Model quad{"res/models/quad.obj", false, false};
    Texture mainTexture;
    mainTexture.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glClearColor(0, 0, 0, 1);
    constexpr unsigned numPerGroup = 20;

    std::thread showFps([&app](){ while(!glfwWindowShouldClose(app.window)) { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); glfwSetWindowTitle(app.window, ("lopengl -- " + std::to_string((int) glm::round(1 / app.deltatime)) + " FPS").c_str()); }});

    while (!glfwWindowShouldClose(app.window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        int prevWidth = -1, prevHeight = -1;
        app.camera.update(app.deltatime);
        glfwGetWindowSize(app.window, &app.camera.width, &app.camera.height);
        if(prevWidth != app.camera.width || prevHeight != app.camera.height) { // resize textures
            mainTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app.camera.width, app.camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
//  =========================================== 
        glViewport(0, 0, app.camera.width, app.camera.height);
        app.shaders[0].bind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glBindImageTexture(0, mainTexture.getRenderID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        glUniform1i(app.shaders[0].getUniform("u_output"), 0);
        glUniform3fv(app.shaders[0].getUniform("u_camera.position"), 1, &app.camera.position.x);
        glUniform3f(app.shaders[0].getUniform("u_camera.forward"), app.camera.getFront().x, app.camera.getFront().y, app.camera.getFront().z);
        glUniform3f(app.shaders[0].getUniform("u_camera.right"), app.camera.getRight().x, app.camera.getRight().y, app.camera.getRight().z);
        glUniform3f(app.shaders[0].getUniform("u_camera.up"), app.camera.getUp().x, app.camera.getUp().y, app.camera.getUp().z);
        glUniform1f(app.shaders[0].getUniform("u_camera.fov"), app.camera.fov);
        glUniform1f(app.shaders[0].getUniform("u_camera.aspect"), (float) app.camera.width / app.camera.height);
        glUniform1f(app.shaders[0].getUniform("u_time"), glfwGetTime());
        glDispatchCompute(app.camera.width / numPerGroup + 1, app.camera.height / numPerGroup + 1, 1);

        app.shaders[1].bind();
        glUniform1i(app.shaders[0].getUniform("u_texture"), 0);
        mainTexture.bind(0);
        quad.getMeshes()[0].va.bind();
        quad.getMeshes()[0].ib.bind();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDrawElements(GL_TRIANGLES, quad.getMeshes()[0].ib.getSize(), GL_UNSIGNED_INT, nullptr);

//  =========================================== 
        imguistuff(app);
        glfwSwapBuffers(app.window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
    showFps.join();
}
/*
c++ be like:
Because the lvalueness or rvalueness of an expression is independent of its type, it’s possible to have lvalues whose type is rvalue reference, and it’s also possible to have rvalues of the type rvalue reference.
*/