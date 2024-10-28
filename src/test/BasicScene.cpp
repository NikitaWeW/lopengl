#include "BasicScene.hpp"
#include "opengl/GlCall.h"
#include "backends/imgui_impl_glfw.h"
#include <stdexcept>

void test::BasicScene::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    bool &mouseLocked = static_cast<test::BasicScene *>(glfwGetWindowUserPointer(window))->camera.mouseLocked;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        mouseLocked = !mouseLocked;
        glfwSetInputMode(window, GLFW_CURSOR, mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}
void test::BasicScene::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    float &fov = static_cast<test::BasicScene *>(glfwGetWindowUserPointer(window))->camera.fov;
    fov -= (float) yoffset;
    if(fov < 1.0f) fov = 1.0f; 
    if(fov > 45.0f) fov = 45.0f; 
}

test::BasicScene::BasicScene(GLFWwindow *window) :
    camera(window, glm::vec3(0), glm::vec3(0))
{
    glfwSetInputMode(window, GLFW_CURSOR, camera.mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

test::BasicScene::~BasicScene() {
    glfwSetWindowUserPointer(window, nullptr);
}

glm::mat4 test::BasicScene::getMVP(glm::mat4 model) { 
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return camera.getProjectionMatrix(width, height) * camera.getViewMatrix() * model;
}

void test::BasicScene::setMVPUniform(Shader &shader, glm::mat4 const &model, std::string const &name) {
    GLCALL(glUniformMatrix4fv(shader.getUniform(name), 1, GL_FALSE, &getMVP(model)[0][0]));
}

void test::BasicScene::onRender(GLFWwindow *window, double deltatime) {
    camera.update(deltatime);
    GLCALL(glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL));
    renderer.Clear(0.05, 0.1, 0.12);
}

void test::BasicScene::onImGuiRender(double deltatime) {
    ImGui::Begin("scene");
    ImGui::Text("delta time: %f", deltatime);
    ImGui::Text("FPS: %f", deltatime ? 1/deltatime : -1);
    ImGui::Checkbox("wireframe", &wireframe);
    ImGui::End();

    ImGui::Begin("camera");
    ImGui::Text("W, A, S, D, E, Q -- move");
    ImGui::Text("<mouse> -- rotate");
    ImGui::Text("<esc> -- (un)capture mouse");
    ImGui::Text("position: (%f; %f; %f)", camera.position.x, camera.position.y, camera.position.z);
    ImGui::Text("rotation: (%f; %f; %f)", camera.rotation.x, camera.rotation.y, camera.rotation.z);
    if(ImGui::Button("reset")) {
        camera.position = glm::vec3(0, 0, 3.5);
        camera.rotation = glm::vec3(-90, 0, 0);
    }
    ImGui::InputFloat("camera speed", &camera.cameraSpeed);
    ImGui::InputFloat("sensitivity", &camera.sensitivity);
    ImGui::End();
}
