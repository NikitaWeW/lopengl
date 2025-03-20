/*
i use this (gcc + ninja)
cmake -S . -B build -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS='-fdiagnostics-color=always -Wall' -G Ninja
cmake --build build && build/main

        +____________+
        /:\         ,:\
       / : \       , : \
      /  :  \     ,  :  \
     /   :   +-----------+
    +....:../:...+   :  /|
    |\   +./.:...`...+ / |
    | \ ,`/  :   :` ,`/  |
    |  \ /`. :   : ` /`  |
    | , +-----------+  ` |
    |,  |   `+...:,.|...`+
    +...|...,'...+  |   /
     \  |  ,     `  |  /
      \ | ,       ` | /
       \|,         `|/
        +___________+

2-Dimensional Representation Of A 3-Dimensional Cross-Section Of A 4-Dimensional Cube
*/

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "logger.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Application.hpp"
#include "random.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Framebuffer.hpp"
#include "opengl/UniformBuffer.hpp"
#include "opengl/Cubemap.hpp"
#include "utils/AABB.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
#include <stdexcept>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

void imguistuff(Application &app);
float lerp(float a, float b, float x) { return a + x * (b - a); }

int main(int argc, char **argv)
{
    Application app; // initialisation
    app.camera = ControllableCamera{app.window, {0, 0, 0}, {-90, 0, 0}};
    app.shaders = {
        {"shaders/raytracing.glsl", true}
    }; // on shader reload contents will be recompiled, if a shader fails it will be restored.

//  =========================================== 

    float vertices[] = {
        // positions        tex coords
        -1.0, -1.0, 0.0,    0.0, 0.0,
         1.0, -1.0, 0.0,    1.0, 0.0,
         1.0,  1.0, 0.0,    1.0, 1.0,
        -1.0,  1.0, 0.0,    0.0, 1.0 
    };
    ShaderProgram HDRshader = ShaderProgram{"shaders/hdr.glsl", true};
    VertexBuffer quadVB = VertexBuffer{vertices, sizeof(vertices)};
    VertexArray quadVA = VertexArray{quadVB, InterleavedVertexBufferLayout{{3, GL_FLOAT}, {2, GL_FLOAT}}};
    
//  =========================================== 

    Model testModel{"res/models/sphere_low_poly.glb"}; // assuming only one mesh
    SSBO indicesSSBO{testModel.getMeshes()[0].indices.size() * sizeof(testModel.getMeshes()[0].indices[0])};
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
        0, 
        testModel.getMeshes()[0].indices.size() * sizeof(testModel.getMeshes()[0].indices[0]), 
        testModel.getMeshes()[0].indices.data());
    SSBO positionsSSBO{testModel.getMeshes()[0].positions.size() * sizeof(testModel.getMeshes()[0].positions[0])};
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
        0,
        testModel.getMeshes()[0].positions.size() * sizeof(testModel.getMeshes()[0].positions[0]), 
        testModel.getMeshes()[0].positions.data());
    SSBO normalsSSBO{testModel.getMeshes()[0].normals.size() * sizeof(testModel.getMeshes()[0].normals[0])};
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
        0,
        testModel.getMeshes()[0].normals.size() * sizeof(testModel.getMeshes()[0].normals[0]), 
        testModel.getMeshes()[0].normals.data());

    AABB testModelAABB;
    for(unsigned index : testModel.getMeshes()[0].indices) {
        testModelAABB.growToInclude(testModel.getMeshes()[0].positions[index]);
    }
    Material testModelMaterial0{
        {0, 0, 0},
        {10, 10, 10}
    };
    Material testModelMaterial1{
        {0.4, 0.5, 0.1},
        {0, 0, 0}
    };

//  =========================================== 

    Texture mainTexture;
    mainTexture.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glClearColor(0, 0, 0, 1);
    constexpr unsigned numPerGroup = 20;

    std::thread showFps([&app](){ while(!glfwWindowShouldClose(app.window)) { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); glfwSetWindowTitle(app.window, ("lopengl -- " + std::to_string((int) glm::round(1 / app.deltatime)) + " FPS").c_str()); }});
    while (!glfwWindowShouldClose(app.window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        int prevWidth = app.camera.width, prevHeight = app.camera.height;
        glm::vec3 prevCamPos = app.camera.position, prevCamRotation = app.camera.rotation;
        float prevFOV = app.camera.fov;
        app.camera.update(app.deltatime);
        glfwGetWindowSize(app.window, &app.camera.width, &app.camera.height);
        if(prevWidth != app.camera.width || prevHeight != app.camera.height || prevCamPos != app.camera.position || prevCamRotation != app.camera.rotation || prevFOV != app.camera.fov) { // resize textures
            mainTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, app.camera.width, app.camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            app.numAccumFrames = 0;
        }
//  =========================================== 
        glViewport(0, 0, app.camera.width, app.camera.height);
        app.shaders[0].bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glBindImageTexture(0, mainTexture.getRenderID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
        
        indicesSSBO.bind(0);
        positionsSSBO.bind(1);
        normalsSSBO.bind(2);
        // glShaderStorageBlockBinding(app.shaders[0].getRenderID(), app.shaders[0].getStorageBlock("indicesSSBO"), 0);
        // glShaderStorageBlockBinding(app.shaders[0].getRenderID(), app.shaders[0].getStorageBlock("positionsSSBO"), 1);
        // glShaderStorageBlockBinding(app.shaders[0].getRenderID(), app.shaders[0].getStorageBlock("normalsSSBO"), 2);
        
        glUniform1i(app.shaders[0].getUniform("u_output"), 0);
        glUniform1ui(app.shaders[0].getUniform("u_numAccumFrames"), app.numAccumFrames);
        glUniform3fv(app.shaders[0].getUniform("u_camera.position"), 1, &app.camera.position.x);
        glUniform3f(app.shaders[0].getUniform("u_camera.forward"), app.camera.getFront().x, app.camera.getFront().y, app.camera.getFront().z);
        glUniform3f(app.shaders[0].getUniform("u_camera.right"), app.camera.getRight().x, app.camera.getRight().y, app.camera.getRight().z);
        glUniform3f(app.shaders[0].getUniform("u_camera.up"), app.camera.getUp().x, app.camera.getUp().y, app.camera.getUp().z);
        glUniform1f(app.shaders[0].getUniform("u_camera.fov"), app.camera.fov);
        glUniform1f(app.shaders[0].getUniform("u_camera.aspect"), (float) app.camera.width / app.camera.height);
        glUniform1ui(app.shaders[0].getUniform("u_modelCount"), 2);
        glUniform1f(app.shaders[0].getUniform("u_time"), glfwGetTime());
        
        testModel.resetMatrix();
        testModel.translate({0, 0, -4});
        glUniform1ui(app.shaders[0].getUniform("u_models[0].indicesCount"), testModel.getMeshes()[0].indices.size());
        glUniform1ui(app.shaders[0].getUniform("u_models[0].indexOffset"), 0);
        glUniform1ui(app.shaders[0].getUniform("u_models[0].vertexOffset"), 0);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_models[0].modelMat"), 1, GL_FALSE, &testModel.getModelMat()[0][0]);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_models[0].normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(testModel.getModelMat()))[0][0]);
        glUniform3fv(app.shaders[0].getUniform("u_models[0].aabb.min"), 1, &testModelAABB.min.x);
        glUniform3fv(app.shaders[0].getUniform("u_models[0].aabb.max"), 1, &testModelAABB.max.x);
        glUniform3fv(app.shaders[0].getUniform("u_models[0].material.color"), 1, &testModelMaterial0.color.x);
        glUniform3fv(app.shaders[0].getUniform("u_models[0].material.emmission"), 1, &testModelMaterial0.emmission.x);
        
        testModel.resetMatrix();
        testModel.translate({0, -11, -4});
        testModel.scale({10, 10, 10});
        glUniform1ui(app.shaders[0].getUniform("u_models[1].indicesCount"), testModel.getMeshes()[0].indices.size());
        glUniform1ui(app.shaders[0].getUniform("u_models[1].indexOffset"), 0);
        glUniform1ui(app.shaders[0].getUniform("u_models[1].vertexOffset"), 0);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_models[1].modelMat"), 1, GL_FALSE, &testModel.getModelMat()[0][0]);
        glUniformMatrix4fv(app.shaders[0].getUniform("u_models[1].normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(testModel.getModelMat()))[0][0]);
        glUniform3fv(app.shaders[0].getUniform("u_models[1].aabb.min"), 1, &testModelAABB.min.x);
        glUniform3fv(app.shaders[0].getUniform("u_models[1].aabb.max"), 1, &testModelAABB.max.x);
        glUniform3fv(app.shaders[0].getUniform("u_models[1].material.color"), 1, &testModelMaterial1.color.x);
        glUniform3fv(app.shaders[0].getUniform("u_models[1].material.emmission"), 1, &testModelMaterial1.emmission.x);

        glDispatchCompute(app.camera.width / numPerGroup + 1, app.camera.height / numPerGroup + 1, 1);
        ++app.numAccumFrames;

//  =========================================== 

        HDRshader.bind();
        glUniform1i(HDRshader.getUniform("u_texture"), 0);
        mainTexture.bind(0);
        quadVA.bind();
        quadVB.bind();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

//  =========================================== 
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    ImGui::ColorEdit3("material 1 color", &testModelMaterial0.color.r);
    ImGui::ColorEdit3("material 1 emmission", &testModelMaterial0.emmission.r);
    ImGui::ColorEdit3("material 2 color", &testModelMaterial1.color.r);
    ImGui::ColorEdit3("material 2 emmission", &testModelMaterial1.emmission.r);
    ImGui::Separator();
    if(ImGui::Button("reset accumulation")) app.numAccumFrames = 0;
    if(ImGui::Button("reload shaders")) app.reloadShaders();

    if(app.failedToReloadShaders) {
        ImGui::OpenPopup("failed to reload shaders!");
        app.failedToReloadShaders = false;
    }
    if(ImGui::BeginPopup("failed to reload shaders!")) {
        ImGui::Text("shader name: %s", app.lastFailedShaderName.c_str());
        ImGui::Separator();
        ImGui::TextWrapped(app.lastFailedShaderLog.c_str());
        ImGui::EndPopup();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
        
//  =========================================== 
        glfwSwapBuffers(app.window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
    showFps.detach();
}
/*
c++ be like:
Because the lvalueness or rvalueness of an expression is independent of its type, it’s possible to have lvalues whose type is rvalue reference, and it’s also possible to have rvalues of the type rvalue reference.
*/