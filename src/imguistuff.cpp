#include "glad/gl.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "Application.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Shader.hpp"
#include "utils/Light.hpp"
#include "logger.h"
/*
        skybox.bind(1);
        // draw the model
        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.models[app.currentModelIndex].m_position);
        app.models[app.currentModelIndex].rotate(app.models[app.currentModelIndex].m_rotation);
        app.models[app.currentModelIndex].scale(app.models[app.currentModelIndex].m_scale);

        glUniform1f(currentShader.getUniform("u_timepoint"), glfwGetTime());
        glUniform1i(currentShader.getUniform("u_skybox"), 1);
        app.textures[app.currentTextureIndex].bind();
        renderer.drawLighting(app.models[app.currentModelIndex], currentShader, camera); 
        app.textures[app.currentTextureIndex].unbind();

        if(app.showNormals) {
            renderer.drawLighting(app.models[app.currentModelIndex], normalShader, camera); 
        }

        if(light.enabled) {
            // draw the light cube
            app.cube.resetMatrix();
            app.cube.translate(light.position);
            app.cube.scale(glm::vec3{0.03125});
            app.plainColorShader.bind();
            glUniform3fv(app.plainColorShader.getUniform("u_color"), 1, &light.color.x);
            renderer.draw(app.cube, app.plainColorShader, camera);
        }

        if(app.skybox) {
            // draw the skybox as last
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);
            glUniform1i(skyboxShader.getUniform("u_skybox"), 1);
            renderer.draw(app.cube, skyboxShader, camera);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
        }
*/

void imguistuff(Application &app, ControllableCamera &cam)
{
    // dont ask why this logic is in imgui stuff (idk)
    // if(app.models[app.currentModelIndex].getFilepath() == "res/models/wall/wall.obj") {
    //     app.currentModelRotation += glm::vec3{app.deltatime * 30, app.deltatime * -10, app.deltatime * 20};
    // }

    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    ImGui::Begin("properties");
    if(app.deltatime <= 1/100) {
        ImGui::Text("! delta time: %f", app.deltatime);
        ImGui::Text("! FPS: %f", app.deltatime ? 1 / app.deltatime : -1);
    } else if(app.deltatime <= 1/30) {
        ImGui::Text("!! delta time: %f", app.deltatime);
        ImGui::Text("!! FPS: %f", app.deltatime ? 1 / app.deltatime : -1);
    } else {
        ImGui::Text("delta time: %f", app.deltatime);
        ImGui::Text("FPS: %f", app.deltatime ? 1 / app.deltatime : -1);
    }
    ImGui::Separator();

    if(app.displayShaders.size() != 0) {
        std::vector<const char *> shaderNames;
        for(unsigned index : app.displayShaders) shaderNames.push_back(app.shaders[index].getFilePath().c_str());
        ImGui::ListBox("shaders", &app.currentShaderIndex, shaderNames.data(), shaderNames.size());
        if(ImGui::Button("reload shaders")) {
            if(!app.reloadShaders()) ImGui::OpenPopup("failed to reload shaders!");
        }
        if(app.failedToReloadShaders) {
            ImGui::OpenPopup("failed to reload shaders!");
            app.failedToReloadShaders = false;
        }
        ImGui::Separator();
    }

    // ImGui::Checkbox("wireframe", &app.wireframe);
    // ImGui::Checkbox("skybox", &app.skybox);
    // ImGui::Checkbox("show normals", &app.showNormals);
    // ImGui::Checkbox("face culling", &app.faceCulling);
    // ImGui::InputFloat("exposure", &app.exposure);
    ImGui::ColorEdit3("clear color", &app.clearColor.x);
    ImGui::Separator();

    // size_t triangles = 0;
    // for(size_t i = 0; i < app.models[app.currentModelIndex].getMeshes().size(); ++i) {
    //     int meshTris = 0;
    //     app.models[app.currentModelIndex].getMeshes()[i].ib.bind();
    //     glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &meshTris);
    //     triangles += meshTris / sizeof(unsigned) / 3;
    // }
    // ImGui::Text("%lu triangles", triangles);
    // ImGui::Text("%lu vertices", triangles * 3);
    // if(app.models.size() > 0) {
    //     std::vector<const char *> modelNames;
    //     for(Model const &model : app.models) modelNames.push_back(model.getFilepath().c_str());
    //     ImGui::ListBox("loaded models", &app.currentModelIndex, modelNames.data(), modelNames.size());
    // }
    // ImGui::Separator();


    // ImGui::Separator();
    // ImGui::DragFloat3("model position", &app.currentModelPosition.x, 0.01f);
    // ImGui::DragFloat3("rotation", &app.currentModelRotation.x, 0.5f);
    // ImGui::DragFloat3("scale", &app.currentModelScale.x, 0.01f);
    // if (ImGui::Button("reset model"))
    // {
    //     app.currentModelPosition = glm::vec3(0);
    //     app.currentModelRotation = glm::vec3(0);
    //     app.currentModelScale = glm::vec3(1);
    // }

    // ImGui::Separator();


    // ImGui::Checkbox("flashlight enabled", &flashlight.enabled);
    // if(flashlight.enabled) {
    //     ImGui::ColorEdit3("flashlight color", &flashlight.color.r);
    //     ImGui::DragFloat("flashlight attenuation", &flashlight.attenuation, 0.001, 0.00001, 5);
    //     if (ImGui::Button("reset flashlight"))
    //     {
    //         flashlight.color = glm::vec3(1.0);
    //         flashlight.attenuation = 0.07f;
    //     }
    //     ImGui::Separator();


    // }
    // ImGui::Checkbox("light enabled", &light.enabled);
    // if(light.enabled) {
    //     ImGui::DragFloat3("light position", &light.position.x, 0.01f);
    //     ImGui::ColorEdit3("light color", &light.color.r);
    //     ImGui::DragFloat("light attenuation", &light.attenuation, 0.001, 0.00001, 5);
    //     if (ImGui::Button("reset light"))
    //     {
    //         light.position = glm::vec3(1, 1, 2);
    //         light.color = glm::vec3(1.0);
    //         flashlight.attenuation = 0.07f;
    //     }
    // }
    // ImGui::Checkbox("sun enabled", &sun.enabled);
    // if(sun.enabled) {
    //     ImGui::DragFloat3("sun direction", &sun.direction.x, 0.01f);
    //     ImGui::ColorEdit3("sun color", &sun.color.r);
    //     if (ImGui::Button("reset sun"))
    //     {
    //         sun.position = {0, 0, 0};
    //         sun.direction = {1, -0.3, 1};
    //         sun.color = glm::vec3(1.0);
    //         flashlight.attenuation = 0.07f;
    //     }
    //     ImGui::Separator();
    // }
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
        ImGui::Text("shader name: %s", app.lastFailedShaderName.c_str());
        ImGui::Separator();
        ImGui::TextWrapped(app.lastFailedShaderLog.c_str());
        ImGui::EndPopup();
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