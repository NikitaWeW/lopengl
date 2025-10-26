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
#include "stb_image.h"

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
    ogl::Program gridShader = ogl::compileShader("shaders/grid");
    ogl::Program planetShader = ogl::compileShader("shaders/basic");
    stbi_set_flip_vertically_on_load(true);
    ogl::Cubemap planetTexture = texture::loadCubemap("res/textures/earth_albedo.png");
    ogl::Cubemap planetNormal = texture::loadCubemap("res/textures/earth_normal.png");

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

        // sunPos = glm::normalize(glm::vec3(glm::cos(time*0.1), 0, glm::sin(time*0.1))) * 100.0f;
        sunPos = glm::normalize(glm::vec3(glm::cos(time), glm::sin(time) * 0.1, glm::sin(time))) * 100.0f;

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        
        glUseProgram(planetShader.id);
        glActiveTexture(GL_TEXTURE0+0); glBindTexture(GL_TEXTURE_CUBE_MAP, planetTexture.id);
        glActiveTexture(GL_TEXTURE0+1); glBindTexture(GL_TEXTURE_CUBE_MAP, planetNormal.id);
        glUniformMatrix4fv(ogl::getUniform(planetShader, "u_viewMat"),       1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(ogl::getUniform(planetShader, "u_projectionMat"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        glUniformMatrix4fv(ogl::getUniform(planetShader, "u_modelMat"),      1, GL_FALSE, glm::value_ptr(mm{}.scale(glm::vec3{planetSize}).get()));

        glBindVertexArray(model.vao.id);
        glDrawArrays(GL_TRIANGLES, 0, model.count);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1.0E-6;
    }

    glfwTerminate();
}
