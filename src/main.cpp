#include <iostream>
#include <chrono>
#include <cassert>
#include <thread>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/io.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "core/Camera.hpp"
#include "core/ogl.hpp"
#include "core/load.hpp"
#include "core/modelMat.hpp"

bool init(GLFWwindow **window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ControllableCamera &camera = *static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        camera.firstCursorMove = true;
        camera.locked = !camera.locked;
    }
}

int main(int argc, char **argv) {
    GLFWwindow* window;
    if(!init(&window)) {
        std::cout << "failed to init!\n";
        return -1;
    }

    float deltatime = 0.1;
    ControllableCamera camera{window};
    camera.position.z = 150;
    camera.speed = 75;
    camera.near = 0.1;
    camera.far = 1000;
    glfwSetWindowUserPointer(window, &camera);
    model::Loader loader;
    auto model = loader.load("res/models/sphere.obj");
    ogl::Program atmosphereShader = ogl::compileShader("shaders/atmosphere");
    ogl::Program gridShader = ogl::compileShader("shaders/grid");
    ogl::Program planetShader = ogl::compileShader("shaders/basic");

    glfwSetKeyCallback(window, key_callback);

    glm::vec3 sunPos{0};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float planetSize = 100;
    const float atmosphereSize = 15;

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        camera.update(deltatime);
        glfwGetWindowSize(window, &camera.width, &camera.height);
        float time = glfwGetTime();

        glViewport(0, 0, camera.width, camera.height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        glUseProgram(gridShader.id);
        glDisable(GL_CULL_FACE);
        glUniformMatrix4fv(ogl::getUniform(gridShader, "u_viewMat"),       1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(ogl::getUniform(gridShader, "u_projectionMat"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        sunPos = glm::normalize(glm::vec3(glm::cos(time), 0, glm::sin(time))) * 10.0f;
        // sunPos = glm::normalize(glm::vec3(glm::cos(time), glm::cos(time * 0.1), glm::sin(time))) * 10.0f;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glUseProgram(planetShader.id);
        glUniformMatrix4fv(ogl::getUniform(planetShader, "u_viewMat"),       1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(ogl::getUniform(planetShader, "u_projectionMat"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        glUniformMatrix4fv(ogl::getUniform(planetShader, "u_modelMat"),      1, GL_FALSE, glm::value_ptr(mm{}.scale(glm::vec3{planetSize}).get()));
        glUniform3fv(      ogl::getUniform(planetShader, "sunPos"), 1, glm::value_ptr(sunPos));

        glBindVertexArray(model.vao.id);
        glDrawArrays(GL_TRIANGLES, 0, model.count);
        
        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(atmosphereShader.id);
        glUniformMatrix4fv(ogl::getUniform(atmosphereShader, "ViewMatrix"),        1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(ogl::getUniform(atmosphereShader, "ProjMat"),           1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        glUniformMatrix4fv(ogl::getUniform(atmosphereShader, "ModelMat"),          1, GL_FALSE, glm::value_ptr(mm{}.scale(glm::vec3{planetSize + atmosphereSize}).get()));
        glUniform3fv(      ogl::getUniform(atmosphereShader, "SS_CameraPosition"), 1, glm::value_ptr(camera.position));
        glUniform3fv(      ogl::getUniform(atmosphereShader, "LightPosition"), 1, glm::value_ptr(sunPos));
        
        glBindVertexArray(model.vao.id);
        glDrawArrays(GL_TRIANGLES, 0, model.count);
        glEnable(GL_DEPTH_TEST);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1.0E-6;
    }

    glfwTerminate();
}
