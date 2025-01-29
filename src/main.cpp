/*
i use this (gcc + ninja)
cmake -S . -B build -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS='-fdiagnostics-color=always -Wall' -G Ninja
cmake --build build && build/main
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
#include "opengl/Renderer.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Framebuffer.hpp"
#include "opengl/UniformBuffer.hpp"
#include "opengl/Cubemap.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <stdexcept>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif
#define SHOW_LOGS         true // for readability
#define LOAD_NOW          true
#define FLIP_TEXTURES     true
#define FLIP_WINING_ORDER true
#define SRGB              true
#define currentShader app.shaders[app.displayShaders[app.currentShaderIndex]]

void imguistuff(Application &app, ControllableCamera &cam, PointLight &light, SpotLight &flashlight, DirectionalLight &sun);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main(int argc, char **argv)
{
    printf("loading...\n"); // TODO: cool progress bar
    Application app;
    GLFWwindow *window = app.window;
    Model lightCube("res/models/cube.obj");
    Model oneSideQuad{"res/models/one_side_quad.obj"};
    ControllableCamera camera(window, {0, 0, 3}, {-90, 0, 0});
    PointLight light;
    DirectionalLight sun;
    SpotLight flashlight;
    Renderer renderer;
    Cubemap skybox("res/textures/skybox1", {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg"});
    
//  =========================================== 

    glfwGetWindowSize(window, &camera.width, &camera.height);

    renderer.getLights().push_back(&flashlight);
    renderer.getLights().push_back(&light);
    renderer.getLights().push_back(&sun);

    app.shaders = {
        {"shaders/basic.glsl",          SHOW_LOGS}, // 0
        {"shaders/lighting.glsl",       SHOW_LOGS}, // 1
        {"shaders/reflection.glsl",     SHOW_LOGS}, // 2
        {"shaders/refraction.glsl",     SHOW_LOGS}, // 3
        {"shaders/explode.glsl",        SHOW_LOGS}, // 4
//       =========================================
        {"shaders/post_process.glsl",   SHOW_LOGS}, // 5
        {"shaders/depth_omnidir.glsl",  SHOW_LOGS}, // 6
        {"shaders/depth_regular.glsl",  SHOW_LOGS}, // 7
        {"shaders/plain_color.glsl",    SHOW_LOGS}, // 8
        {"shaders/skybox.glsl",         SHOW_LOGS}, // 9
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 
    app.displayShaders = {0, 1, 2, 3, 4}; // shows in shader list.

    flashlight.position  = camera.position;
    flashlight.direction = camera.getFront();
    light.position= glm::vec3{2, 1, 0};
    sun.direction = glm::vec3{1, -0.5f, 0.5f};

    flashlight.enabled = false;
    light.enabled =      true;
    sun.enabled =        true;

    app.quad = Model{"res/models/quad.obj"};
    app.cube = Model{"res/models/cube.obj"};

//   ==================================================================
    app.models = {
        {"res/models/cube.obj",                            FLIP_TEXTURES,  FLIP_WINING_ORDER },
        {"res/models/sphere/scene.gltf",                   FLIP_TEXTURES, !FLIP_WINING_ORDER },
        {"res/models/lemon/lemon_4k.gltf",                 FLIP_TEXTURES, !FLIP_WINING_ORDER },
        {"res/models/apple/food_apple_01_4k.gltf",         FLIP_TEXTURES, !FLIP_WINING_ORDER },
        {"res/models/backpack/backpack.obj",              !FLIP_TEXTURES, !FLIP_WINING_ORDER },
    };
    // Texture concreteTexture("res/textures/concrete.jpg",   FLIP_TEXTURES, SRGB, GL_REPEAT);
    Texture oakTexture("res/textures/oak.jpg",             FLIP_TEXTURES, SRGB, GL_REPEAT);

// =========================== //

    app.currentModelIndex = 1;    // sphere
    app.currentShaderIndex = 1;   // lighting

// =========================== //

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_CURSOR, camera.locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    LOG_INFO("loaded!");

// =========================== //

    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };
    VertexBuffer planeVBO{planeVertices, sizeof(planeVertices)};
    InterleavedVertexBufferLayout planeVBLayout{
        {3, GL_FLOAT},
        {3, GL_FLOAT},
        {2, GL_FLOAT}
    };
    VertexArray planeVAO{};
    planeVAO.addBuffer(planeVBO, planeVBLayout);

// ============================ //
//     generate a depth map     //
// ============================ //

    const unsigned SHADOW_RESOLUTION = 2048;
    
// =========================== //

    Cubemap lightDepthMap{GL_CLAMP_TO_EDGE, GL_NEAREST};
    lightDepthMap.bind();
    for(unsigned i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    Framebuffer lightDepthMapFBO;
    lightDepthMapFBO.bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMap.getRenderID(), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    lightDepthMapFBO.unbind();
    assert(lightDepthMapFBO.isComplete());

// =========================== //

    Texture sunDepthMap{GL_CLAMP_TO_BORDER};
    sunDepthMap.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    Framebuffer sunDepthMapFBO;
    sunDepthMapFBO.bind();
    sunDepthMapFBO.attach(sunDepthMap, GL_DEPTH_ATTACHMENT);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    sunDepthMapFBO.unbind();
    assert(sunDepthMapFBO.isComplete());

// =========================== //

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        camera.update(app.deltatime);
        flashlight.position  = camera.position;
        flashlight.direction = camera.getFront();
        int lastWidth = camera.width, lastHeight = camera.height;
        glfwGetWindowSize(window, &camera.width, &camera.height);

// ============================ //
//     draw to the depth map    //
// ============================ //
// TODO: draw the depth map in renderer

// ===================== //
//         light         //
// ===================== //

        glm::mat4 shadowTransformations[] = {
            light.getProjectionMatrix() * glm::lookAt(light.position, light.position + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),
            light.getProjectionMatrix() * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),
            light.getProjectionMatrix() * glm::lookAt(light.position, light.position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
            light.getProjectionMatrix() * glm::lookAt(light.position, light.position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)),
            light.getProjectionMatrix() * glm::lookAt(light.position, light.position + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)),
            light.getProjectionMatrix() * glm::lookAt(light.position, light.position + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0))
        };
        lightDepthMapFBO.bind();
        app.shaders[6].bind();
        glUniform3fv(app.shaders[6].getUniform("u_light.position"), 1, &light.position.x);
        for(int i = 0; i < 6; ++i)
            glUniformMatrix4fv(app.shaders[6].getUniform("u_shadowMatrices[" + std::to_string(i) + "]"), 1, GL_FALSE, &shadowTransformations[i][0][0]);

        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        renderer.clear();
        glDisable(GL_CULL_FACE);

        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.currentModelPosition);
        app.models[app.currentModelIndex].rotate(app.currentModelRotation);
        app.models[app.currentModelIndex].scale(app.currentModelScale);

        glUniformMatrix4fv(app.shaders[6].getUniform("u_modelMat"), 1, GL_FALSE, &app.models[app.currentModelIndex].getModelMat()[0][0]);
        renderer.draw(app.models[app.currentModelIndex]); 

        glDisable(GL_CULL_FACE);
        glUniformMatrix4fv(app.shaders[6].getUniform("u_modelMat"), 1, GL_FALSE, &glm::mat4{1}[0][0]);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        glFrontFace(GL_CW);
        oakTexture.bind(1);
        app.cube.resetMatrix();
        app.cube.translate({-1.5f, 1.0f, 1.5});
        app.cube.scale(glm::vec3{0.75f});
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
        renderer.draw(app.cube); 
        glFrontFace(GL_CCW);

        glEnable(GL_CULL_FACE);

// ===================== //
//          sun          //
// ===================== //

        glCullFace(GL_FRONT);
        sunDepthMapFBO.bind();
        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        renderer.clear();

        app.shaders[7].bind();
        glUniformMatrix4fv(app.shaders[7].getUniform("u_viewMat"),      1, GL_FALSE, &sun.getViewMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[7].getUniform("u_projectionMat"),1, GL_FALSE, &sun.getProjectionMatrix()[0][0]);

        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.currentModelPosition);
        app.models[app.currentModelIndex].rotate(app.currentModelRotation);
        app.models[app.currentModelIndex].scale(app.currentModelScale);

        glUniformMatrix4fv(app.shaders[7].getUniform("u_modelMat"), 1, GL_FALSE, &app.models[app.currentModelIndex].getModelMat()[0][0]);
        renderer.draw(app.models[app.currentModelIndex]); 

        glDisable(GL_CULL_FACE);
        glUniformMatrix4fv(app.shaders[7].getUniform("u_modelMat"), 1, GL_FALSE, &glm::mat4{1}[0][0]);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        
        glFrontFace(GL_CW);
        oakTexture.bind(1);
        app.cube.resetMatrix();
        app.cube.translate({-1.5f, 1.0f, 1.5});
        app.cube.scale(glm::vec3{0.75f});
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
        renderer.draw(app.cube); 
        glFrontFace(GL_CCW);

// ===================== //
//     draw the scene    //
// ===================== //

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear(app.clearColor);

        lightDepthMap.bind(0);
        sunDepthMap.bind(1);
        oakTexture.bind(2);
        currentShader.bind();
        glUniform3fv(currentShader.getUniform("u_viewPos"), 1, &camera.position.x);
        glUniform1i(currentShader.getUniform("u_pointLights[0].depthMap"),   0);
        glUniform1i(currentShader.getUniform("u_dirLights[0].depthMap"),   1);
        glUniformMatrix4fv(currentShader.getUniform("u_viewMat"),      1, GL_FALSE, &camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
        glUniform1i(currentShader.getUniform("u_material.diffuse"), 2);
        glUniform1i(currentShader.getUniform("u_material.specular"), 3);
        glUniform1f(currentShader.getUniform("u_material.shininess"), 32);
        glUniform1i(currentShader.getUniform("u_specularSet"), false);
        renderer.setLightingUniforms(currentShader);

// ================== //

        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.currentModelPosition);
        app.models[app.currentModelIndex].rotate(app.currentModelRotation);
        app.models[app.currentModelIndex].scale(app.currentModelScale);
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &app.models[app.currentModelIndex].getModelMat()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(app.models[app.currentModelIndex].getModelMat()))[0][0]);
        renderer.draw(app.models[app.currentModelIndex]); 

        glDisable(GL_CULL_FACE);
        glm::mat4 planeModelMat = glm::mat4{1};
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &planeModelMat[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(planeModelMat))[0][0]);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glFrontFace(GL_CW);
        app.cube.resetMatrix();
        app.cube.translate({-1.5f, 1.0f, 1.5});
        app.cube.scale(glm::vec3{0.75f});
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(app.cube.getModelMat()))[0][0]);
        renderer.draw(app.cube); 
        glFrontFace(GL_CCW);

// ================== //
        
        if(light.enabled) {
            // draw the light cube
            app.cube.resetMatrix();
            app.cube.translate(light.position);
            app.cube.scale(glm::vec3{0.03125});
            app.shaders[8].bind();
            glUniform3fv(app.shaders[8].getUniform("u_color"), 1, &light.color.x);
            glUniformMatrix4fv(app.shaders[8].getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
            glUniformMatrix4fv(app.shaders[8].getUniform("u_viewMat"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
            glUniformMatrix4fv(app.shaders[8].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
            renderer.draw(app.cube);
        } 

// ================== //

        imguistuff(app, camera, light, flashlight, sun);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
}
/*
c++ be like:
Because the lvalueness or rvalueness of an expression is independent of its type, it’s possible to have lvalues whose type is rvalue reference, and it’s also possible to have rvalues of the type rvalue reference.
*/