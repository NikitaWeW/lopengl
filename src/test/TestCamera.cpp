#include "TestCamera.hpp"
#include "glfw/glfw3.h"
#include "opengl/GlCall.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.hpp"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"
#include <stdexcept>

void test::TestCamera::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    test::TestCamera *thistest = static_cast<test::TestCamera *>(glfwGetWindowUserPointer(window));
    thistest->cam.fov -= (float) yoffset;
    if(thistest->cam.fov < 1.0f) thistest->cam.fov = 1.0f; 
    if(thistest->cam.fov > 45.0f) thistest->cam.fov = 45.0f; 
}
void test::TestCamera::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    test::TestCamera *thistest = static_cast<test::TestCamera *>(glfwGetWindowUserPointer(window));
    if(action == GLFW_PRESS) {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            thistest->mouseLocked = !thistest->mouseLocked;
            if(thistest->mouseLocked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            break;
        default:
            break;
        }
    }
}
void test::TestCamera::cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    test::TestCamera *thistest = static_cast<test::TestCamera *>(glfwGetWindowUserPointer(window));
    if(thistest->mouseLocked) {
        if (thistest->firstCursorMove)
        {
            thistest->prevx = xpos;
            thistest->prevy = ypos;
            thistest->firstCursorMove = false;
        }

        float xoffset = xpos - thistest->prevx;
        float yoffset = thistest->prevy - ypos; // reversed since y-coordinates go from bottom to top

        thistest->prevx = xpos;
        thistest->prevy = ypos;

        xoffset *= thistest->sensitivity;
        yoffset *= thistest->sensitivity;

        thistest->cam.rotation.x += xoffset;
        thistest->cam.rotation.y += yoffset;

        if (thistest->cam.rotation.y > 89.0f)
            thistest->cam.rotation.y = 89.0f;
        if (thistest->cam.rotation.y < -89.0f)
            thistest->cam.rotation.y = -89.0f;
        
        thistest->cam.update();
    }
}

test::TestCamera::~TestCamera() {
    LOG_INFO("destroying test");
}
test::TestCamera::TestCamera(GLFWwindow *window) : 
    cam(glm::vec3(0, 0, 2.5), glm::vec3(-90, 0, 0)),
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, test::TestCamera::key_callback);
    glfwSetCursorPosCallback(window, test::TestCamera::cursor_position_callback);
    glfwSetScrollCallback(window, test::TestCamera::scroll_callback);

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
void test::TestCamera::processCameraMovement(GLFWwindow *window, double deltatime) {
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.position += cameraSpeed * cam.getFront() * (float) deltatime;
    } 
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.position -= cameraSpeed * glm::cross(cam.getFront(), cam.getUp()) * (float) deltatime;
    } 
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.position -= cameraSpeed * cam.getFront() * (float) deltatime;
    } 
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.position += cameraSpeed * glm::cross(cam.getFront(), cam.getUp()) * (float) deltatime;
    }
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cam.position += cameraSpeed * cam.getUp() * (float) deltatime;
    }
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cam.position += cameraSpeed * -cam.getUp() * (float) deltatime;
    }
}
void test::TestCamera::onRender(GLFWwindow *window, double deltatime) {
    glfwSetWindowUserPointer(window, this);
    processCameraMovement(window, deltatime);
    cam.update();
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
    ImGui::Text("delta time: %f", &deltatime);
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
        cam.position = glm::vec3(0, 0, 2.5);
        cam.rotation = glm::vec3(-90, 0, 0);
    }
    ImGui::InputFloat("camera speed", &cameraSpeed);
    ImGui::InputFloat("sensitivity", &sensitivity);
    ImGui::End();
}
