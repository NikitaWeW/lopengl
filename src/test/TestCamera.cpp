#include "TestCamera.hpp"
#include "glfw/glfw3.h"
#include "opengl/GlCall.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.hpp"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"
#include <stdexcept>

void test::TestCamera::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    bool &mouseLocked = static_cast<TestCamera *>(glfwGetWindowUserPointer(window))->cam.mouseLocked;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        mouseLocked = !mouseLocked;
        glfwSetInputMode(window, GLFW_CURSOR, mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}
void test::TestCamera::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    float &fov = static_cast<test::TestCamera *>(glfwGetWindowUserPointer(window))->cam.fov;
    fov -= (float) yoffset;
    if(fov < 1.0f) fov = 1.0f; 
    if(fov > 45.0f) fov = 45.0f; 
}


test::TestCamera::~TestCamera() {
    LOG_INFO("destroying test");
}
test::TestCamera::TestCamera(GLFWwindow *window) : 
    cam(glm::vec3(0, 0, 3.5), glm::vec3(-90, 0, 0), window),
    VB(test::squareVertices, sizeof(test::squareVertices)),
    IB(test::squareIndicies, 6),
    brickWallTexture("res/textures/wall.png"),
    translation1(0),
    rotation1(0),
    scale1(1),
    translation2(0),
    rotation2(0),
    scale2(1) 
{
    LOG_INFO("creating test");
    glfwSetInputMode(window, GLFW_CURSOR, cam.mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if(!shader.ParceShaderFile("src/basic.glsl")) throw std::runtime_error("failed to parce shaders!");
    if(!shader.CompileShaders()) throw std::runtime_error("failed to compile shaders!");

    layout.push<float>(3);
    layout.push<float>(2);
    shader.bind();
    brickWallTexture.bind();
    color1UniformLocation = shader.getUniform("u_color");
    GLCALL(glUniform1i(shader.getUniform("u_Texture"), 0));
    VA.bind();  
    VA.addBuffer(VB, layout);
}

void test::TestCamera::onRender(GLFWwindow *window, double deltatime) {
    cam.update(deltatime);
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::mat4 proj = cam.getProjectionMatrix(windowWidth, windowHeight);
    glm::mat4 view = cam.getViewMatrix();

    if(wireframe) {
        GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    } else {
        GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
    renderer.Clear(0.05, 0.1, 0.12);
    GLCALL(glUniform4f(color1UniformLocation, color[0], color[1], color[2], color[3]));

    { // object 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, translation1);
        model = glm::rotate(model, glm::radians(rotation1.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation1.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation1.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale1);
        glm::mat4  MVP = proj * view * model;
        GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));
        renderer.Draw(VA, IB, shader);
    }
    if(object2) { // object 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, translation2);
        model = glm::rotate(model, glm::radians(rotation2.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation2.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation2.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale2);
        glm::mat4  MVP = proj * view * model;
        GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));
        renderer.Draw(VA, IB, shader);
    }
}
void test::TestCamera::onImGuiRender(double deltatime) {
    ImGui::Begin("camera test");
    ImGui::Text("W, A, S, D, E, Q -- move");
    ImGui::Text("<mouse> -- rotate");
    ImGui::Text("<esc> -- (un)capture mouse");
    ImGui::Text("delta time: %f", deltatime);
    ImGui::Text("FPS: %f", deltatime ? 1/deltatime : -1);
    ImGui::ColorEdit4("color", color);
    ImGui::Checkbox("wireframe", &wireframe);
    ImGui::Checkbox("object 2", &object2);
    if(ImGui::Combo("texture", &current_texture_item, textureNames, IM_ARRAYSIZE(textureNames))) {
        switch (current_texture_item) {
        case 0:
            brickWallTexture.bind();
            break;
        default:
            brickWallTexture.unbind();
            break;
        }
    }
    ImGui::End();

    ImGui::Begin("square 1");
    ImGui::DragFloat3("position", &translation1.x, 0.01f);
    ImGui::DragFloat3("rotation", &rotation1.x, 0.5f);
    ImGui::DragFloat3("scale", &scale1.x, 0.01f);
    if(ImGui::Button("reset")) {
        translation1 = glm::vec3(0);
        rotation1 = glm::vec3(0);
        scale1 = glm::vec3(1);
    }
    ImGui::End();

    if(object2) {
        ImGui::Begin("square 2");
        ImGui::DragFloat3("position", &translation2.x, 0.01f);
        ImGui::DragFloat3("rotation", &rotation2.x, 0.5f);
        ImGui::DragFloat3("scale", &scale2.x, 0.01f);
        if(ImGui::Button("reset")) {
            translation2 = glm::vec3(0);
            rotation2 = glm::vec3(0);
            scale2 = glm::vec3(1);
        }
        ImGui::End();
    }

    ImGui::Begin("view");
    ImGui::Text("position: (%f; %f; %f)", cam.position.x, cam.position.y, cam.position.z);
    ImGui::Text("rotation: (%f; %f; %f)", cam.rotation.x, cam.rotation.y, cam.rotation.z);
    if(ImGui::Button("reset")) {
        cam.position = glm::vec3(0, 0, 3.5);
        cam.rotation = glm::vec3(-90, 0, 0);
    }
    ImGui::InputFloat("camera speed", &cam.cameraSpeed);
    ImGui::InputFloat("sensitivity", &cam.sensitivity);
    ImGui::End();
}
