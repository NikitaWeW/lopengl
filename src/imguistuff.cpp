#include "glad/gl.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "Application.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Shader.hpp"
#include "utils/Light.hpp"
#include "logger.h"

void imguistuff(Application &app, ControllableCamera &cam, PointLight &light, SpotLight &flashlight)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("properties");
    ImGui::Text("delta time: %f", app.deltatime);
    ImGui::Text("FPS: %f", app.deltatime ? 1 / app.deltatime : -1);
    ImGui::Separator();
    std::vector<const char *> shaderNames;
    for(Shader &shader : app.shaders) shaderNames.push_back(shader.getFilePath().c_str());
    ImGui::ListBox("shaders", &app.currentShaderIndex, shaderNames.data(), shaderNames.size());
    static Shader *failedShaderTemp = nullptr;
    if(ImGui::Button("reload shaders")) {
        for(Shader &shader : app.shaders) {
            Shader copy = shader;
            if(!shader.ParceShaderFile(shader.getFilePath())) {
                shader.swap(std::forward<Shader>(copy));
                failedShaderTemp = &shader;
                ImGui::OpenPopup("failed to reload shaders!");
                break;
            };
            if(!shader.CompileShaders()) {
                shader.swap(std::forward<Shader>(copy));
                failedShaderTemp = &shader;
                ImGui::OpenPopup("failed to reload shaders!");
                break;
            }
        }
    }
    ImGui::Separator();
    if(ImGui::Checkbox("wireframe", &app.wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, app.wireframe ? GL_LINE : GL_FILL);
    }
    ImGui::ColorEdit4("clear color", &app.clearColor.x);
    ImGui::Separator();
    if(app.currentModel) {
        size_t triangles = 0;
        for(size_t i = 0; i < app.currentModel->getMeshes().size(); ++i) {
            triangles += app.currentModel->getMeshes()[i].indices.size() / 3;
        }
        ImGui::Text("%u triangles", triangles);
    }
    if(app.modelNames.size() > 0) {
        std::vector<const char *> modelCNames;
        for(std::string &name : app.modelNames) modelCNames.push_back(name.c_str());
        if(ImGui::ListBox("loaded models", &app.currentModelIndex, modelCNames.data(), modelCNames.size())) {
            app.applyModel();
        }
    }
    ImGui::InputText("model path", app.loadModelBuffer, sizeof(app.loadModelBuffer));
    if(ImGui::Button("load model")) {
        app.addModel(app.loadModelBuffer);
    }
    ImGui::Separator();
    if(app.modelNames.size() > 0) {
        std::vector<const char *> textureCNames;
        for(std::string const &name : app.textureNames) textureCNames.push_back(name.c_str());
        if(ImGui::ListBox("loaded textures", &app.currentTextureIndex, textureCNames.data(), textureCNames.size())) {
            app.applyTexture();
        }
    }
    ImGui::InputText("texture path", app.loadTextureBuffer, sizeof(app.loadTextureBuffer));
    if(ImGui::Button("load texture")) {
        app.addTexture(app.loadTextureBuffer);
    }
    ImGui::Separator();
    // ImGui::Text("model");
    // ImGui::DragFloat3("model position", &app.currentModel->m_position.x, 0.01f);
    // ImGui::DragFloat3("rotation", &app.currentModel->m_rotation.x, 0.5f);
    // ImGui::DragFloat3("scale", &app.currentModel->m_scale.x, 0.01f);
    // ImGui::InputFloat3("rotation per ms", &app.cuberotation.x);
    // if (ImGui::Button("reset model"))
    // {
    //     app.currentModel->m_position = glm::vec3(0);
    //     app.currentModel->m_rotation = glm::vec3(0);
    //     app.currentModel->m_scale = glm::vec3(1);
    //     app.cuberotation = glm::vec3(0);
    // }
    // ImGui::Separator();
    ImGui::Text("flashlight");
    ImGui::Checkbox("flashlight enabled", &flashlight.enabled);
    ImGui::ColorEdit3("flashlight color", &flashlight.color.r);
    ImGui::DragFloat("flashlight constant attenuation", &flashlight.constant, 0.01, 0, 50);
    ImGui::DragFloat("flashlight linear attenuation", &flashlight.linear, 0.01, 0, 50);
    ImGui::DragFloat("flashlight quadratic attenuation", &flashlight.quadratic, 0.01, 0, 50);
    if (ImGui::Button("reset flashlight"))
    {
        flashlight.color = glm::vec3(1.0);
        flashlight.constant = 1.0f;
        flashlight.linear = 0.14f;
        flashlight.quadratic = 0.07f;
    }
    ImGui::Text("light");
    ImGui::Checkbox("light enabled", &light.enabled);
    ImGui::DragFloat3("light position / direction", &light.position.x, 0.01f);
    ImGui::ColorEdit3("light color", &light.color.r);
    ImGui::DragFloat("light constant attenuation", &light.constant, 0.01, 0, 50);
    ImGui::DragFloat("light linear attenuation", &light.linear, 0.01, 0, 50);
    ImGui::DragFloat("light quadratic attenuation", &light.quadratic, 0.01, 0, 50);
    if (ImGui::Button("reset light"))
    {
        light.position = glm::vec3(2, 1, 3);
        light.color = glm::vec3(1.0);
        light.constant = 1.0f;
        light.linear = 0.14f;
        light.quadratic = 0.07f;
    }
    ImGui::Separator();
    ImGui::DragFloat3("camera position", &cam.position.x, 0.01f);
    ImGui::DragFloat3("camera rotation", &cam.rotation.x, 0.5f);
    ImGui::DragFloat("camera near plane", &cam.near, 0.001f);
    ImGui::DragFloat("camera far plane", &cam.far, 25.0f);
    ImGui::InputFloat("camera speed (unit/s)", &cam.speed);
    ImGui::InputFloat("camera sensitivity", &cam.sensitivity);
    if (ImGui::Button("reset camera"))
    {
        cam.position = {0, 0, 5};
        cam.rotation = {-90, 0, 0};
        cam.fov = 45;
        cam.near = 0.01f;
        cam.far = 1000.0f;
        cam.speed = 7.0f;
        cam.sensitivity = 1.0f;
    }
    if(ImGui::BeginPopup("failed to reload shaders!")) {
        if(failedShaderTemp) {
            ImGui::Text("shader name: %s", failedShaderTemp->getFilePath().c_str());
            ImGui::Separator();
            ImGui::TextWrapped(failedShaderTemp->getLog().c_str());
            ImGui::Separator();
        } else {
            ImGui::Text("no informaion :(");
        }
        ImGui::EndPopup();
    } else {
        failedShaderTemp = nullptr;
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}