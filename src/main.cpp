#include <iostream>
#include <chrono>
#include <cassert>
#include <thread>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "core/Camera.hpp"
#include "core/ogl/Shader.hpp"
#include "core/ogl/VertexBuffer.hpp"
#include "core/ogl/Texture.hpp"
#include "core/load.hpp"
#include "core/modelMat.hpp"

bool init(GLFWwindow **window);

class Deallocator {
public: 
    inline ~Deallocator() {
        glfwTerminate();
    }
};

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
    std::unique_ptr<Deallocator> cleanup = std::make_unique<Deallocator>();
    GLFWwindow* window;
    if(!init(&window)) {
        std::cout << "failed to init!\n";
        return -1;
    }

    double deltatime = 0.1;
    ControllableCamera camera{window};
    camera.position.z = 4;
    glfwSetWindowUserPointer(window, &camera);
    model::Loader loader;
    auto cube = loader.load("res/models/cube.obj");
    ogl::ShaderProgram shader{"shaders/basic"};
    ogl::ShaderProgram gridShader{"shaders/grid"};

    glfwSetKeyCallback(window, key_callback);

    ogl::Texture texture = texture::load("res/textures/wood0/wood_planks_diff_2k.png", "diffuse");
    std::thread fpsShower{[](GLFWwindow *window, double const *deltatime){
        assert(window);
        assert(deltatime);
        while(!glfwWindowShouldClose(window))
        {
            glfwSetWindowTitle(window, ("ogl setup | " + std::to_string(*deltatime * 1e3) + "ms").c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds{250});
        }
    }, window, &deltatime};

    glm::vec3 rotation{0};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        camera.update(deltatime);
        glfwGetWindowSize(window, &camera.width, &camera.height);

        glViewport(0, 0, camera.width, camera.height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        rotation += float(deltatime) * glm::vec3{1, 2, 3};
        glm::mat4 modelMat = mm{}.translate(1, 1, 0).rotate(rotation).get();

        shader.bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.getRenderID());
        glUniformMatrix4fv(shader.getUniform("u_viewMat"),       1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(shader.getUniform("u_projectionMat"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        glUniformMatrix4fv(shader.getUniform("u_modelMat"),      1, GL_FALSE, glm::value_ptr(modelMat));

        glBindVertexArray(cube.vao.getRenderID());
        glDrawArrays(GL_TRIANGLES, 0, cube.count);
        
        glUseProgram(gridShader.getRenderID());
        glUniformMatrix4fv(gridShader.getUniform("u_viewMat"),       1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(gridShader.getUniform("u_projectionMat"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1.0E-6;
    }

    fpsShower.join();
}
