#include "BasicScene.hpp"
#include "opengl/GlCall.h"
#include "backends/imgui_impl_glfw.h"
#include <stdexcept>

void test::BasicScene::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    ControllableCamera &camera = static_cast<test::BasicScene *>(glfwGetWindowUserPointer(window))->camera;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        camera.mouseLocked = !camera.mouseLocked;
        if(camera.mouseLocked) {
            camera.firstCursorMove = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
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
    renderer.Clear(clearColor.r, clearColor.g, clearColor.b);
}

void test::BasicScene::onImGuiRender(double deltatime) {
    ImGui::Begin("properties");
    ImGui::Separator();
    ImGui::Checkbox("wireframe", &wireframe);
    ImGui::ColorEdit4("color", &clearColor.r);
    ImGui::End();

    ImGui::Begin("camera");
    ImGui::Text("W, A, S, D, E, Q -- move");
    ImGui::Text("<mouse> -- rotate");
    ImGui::Text("<esc> -- (un)capture mouse");
    ImGui::Separator();
    ImGui::Text("position: (%f; %f; %f)", camera.position.x, camera.position.y, camera.position.z);
    ImGui::Text("rotation: (%f; %f; %f)", camera.rotation.x, camera.rotation.y, camera.rotation.z);
    ImGui::Separator();
    if(ImGui::Button("reset position")) {
        camera.position = glm::vec3(0, 0, 3.5);
        camera.rotation = glm::vec3(-90, 0, 0);
    }
    ImGui::Separator();
    ImGui::InputFloat("camera speed", &camera.cameraSpeed);
    ImGui::InputFloat("sensitivity", &camera.sensitivity);
    ImGui::End();
}
