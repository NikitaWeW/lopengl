/*
impliment big imgui code here  
*/
#include "glad/gl.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "Application.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Shader.hpp"
#include "utils/Light.hpp"
#include "logger.h"

void imguistuff(Application &app, ControllableCamera &cam, std::vector<Shader *> shaders, Light &light)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("properties");
    ImGui::Text("delta time: %f", app.deltatime);
    ImGui::Text("FPS: %f", app.deltatime ? 1 / app.deltatime : -1);
    ImGui::Separator();
    if(ImGui::Button("recompile shaders")) {
        for(Shader *shader : shaders) {
            Shader copy = *shader;
            LOG_INFO("recompiling %s...", shader->getFilePath().c_str());
            if(!shader->ParceShaderFile(shader->getFilePath())) {
                shader->swap(std::forward<Shader>(copy));
                continue;
            };
            if(!shader->CompileShaders()) {
                shader->swap(std::forward<Shader>(copy));
                continue;
            }
        }
        LOG_INFO("done.");
    }
    ImGui::Separator();
    if(ImGui::Checkbox("wireframe", &app.wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, app.wireframe ? GL_LINE : GL_FILL);
    }
    // ImGui::Checkbox("move light", &app.moveLight);
    // ImGui::DragFloat3("light position begin", &app.lightPosBegin.x, 0.01f);
    // ImGui::DragFloat3("light position end", &app.lightPosEnd.x, 0.01f);
    // ImGui::InputFloat("light speed", &app.lightSpeed);
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
             app.currentModel = &app.models.at(app.currentModelIndex);
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
            app.currentTexture = &app.textures[app.currentTextureIndex];
        }
    }
    ImGui::InputText("texture path", app.loadTextureBuffer, sizeof(app.loadTextureBuffer));
    if(ImGui::Button("load texture")) {
        app.addTexture(app.loadTextureBuffer);
    }
    ImGui::Separator();
    ImGui::Text("model");
    ImGui::DragFloat3("model position", &app.currentModel->m_position.x, 0.01f);
    ImGui::DragFloat3("rotation", &app.currentModel->m_rotation.x, 0.5f);
    ImGui::DragFloat3("scale", &app.currentModel->m_scale.x, 0.01f);
    ImGui::InputFloat3("rotation per ms", &app.cuberotation.x);
    if (ImGui::Button("reset model"))
    {
        app.currentModel->m_position = glm::vec3(0);
        app.currentModel->m_rotation = glm::vec3(0);
        app.currentModel->m_scale = glm::vec3(1);
        app.cuberotation = glm::vec3(0);
    }
    ImGui::Separator();
    ImGui::Text("light");
    ImGui::DragFloat3("light position / direction", &light.position.x, 0.01f);
    ImGui::ColorEdit3("light ambient", &light.ambient.r);
    ImGui::ColorEdit3("light diffuse", &light.diffuse.r);
    ImGui::ColorEdit3("light specular", &light.specular.r);
    if (ImGui::Button("reset light"))
    {
        light.position = glm::vec3(2, 1, 3);
        light.ambient = glm::vec3(0.1);
        light.diffuse = glm::vec3(0.5);
        light.specular = glm::vec3(1.0);
    }
    ImGui::Separator();
    ImGui::DragFloat3("camera position", &cam.position.x, 0.01f);
    ImGui::DragFloat3("camera rotation", &cam.rotation.x, 0.5f);
    ImGui::DragFloat("camera near plane", &cam.near, 0.001f);
    ImGui::DragFloat("camera far plane", &cam.far, 25.0f);
    ImGui::InputFloat("camera speed", &cam.cameraSpeed);
    ImGui::InputFloat("camera sensitivity", &cam.sensitivity);
    if (ImGui::Button("reset camera"))
    {
        cam.position = {0, 0, 5};
        cam.rotation = {-90, 0, 0};
        cam.fov = 45;
        cam.near = 0.01f;
        cam.far = 1000.0f;
        cam.cameraSpeed = 1.0f;
        cam.sensitivity = 1.0f;
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