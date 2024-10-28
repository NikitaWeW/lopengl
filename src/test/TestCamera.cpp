#include "TestCamera.hpp"
#include "glfw/glfw3.h"
#include "opengl/GlCall.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.hpp"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"
#include <stdexcept>

test::TestCamera::~TestCamera() {
    LOG_INFO("destroying test");
}
test::TestCamera::TestCamera(GLFWwindow *window) : 
    BasicScene(window),
    VB(test::squareVertices, sizeof(test::squareVertices)),
    IB(test::squareIndicies, 6),
    brickWallTexture("res/textures/wall.png")
{
    LOG_INFO("creating test");

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
    BasicScene::onRender(window, deltatime);
    GLCALL(glUniform4f(color1UniformLocation, color[0], color[1], color[2], color[3]));

    { // object 1
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, translation1);
        model = glm::rotate(model, glm::radians(rotation1.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation1.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation1.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale1);
        setMVPUniform(shader, model);
        renderer.Draw(VA, IB, shader);
    }
    if(object2) { // object 2
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, translation2);
        model = glm::rotate(model, glm::radians(rotation2.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation2.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation2.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale2);
        setMVPUniform(shader, model);
        renderer.Draw(VA, IB, shader);
    }
}
void test::TestCamera::onImGuiRender(double deltatime) {
    BasicScene::onImGuiRender(deltatime);
    ImGui::Begin("camera test");
    ImGui::ColorEdit4("color", color);
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
}
