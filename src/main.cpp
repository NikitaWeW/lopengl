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
    ControllableCamera camera(window, {0, 0, 7}, {-90, 0, 0});
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
        {"shaders/basic.glsl",          SHOW_LOGS},
        {"shaders/lighting.glsl",       SHOW_LOGS},
        {"shaders/reflection.glsl",     SHOW_LOGS},
        {"shaders/refraction.glsl",     SHOW_LOGS},
        {"shaders/explode.glsl",        SHOW_LOGS},
//       =========================================
        {"shaders/post_process.glsl",   SHOW_LOGS},
        {"shaders/depth.glsl",          SHOW_LOGS}
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 
    app.displayShaders = {0, 1, 2, 3, 4}; // shows in shader list.

    flashlight.position  = camera.position;
    flashlight.direction = camera.getFront();
    flashlight.enabled = false;

    light.position= glm::vec3{2, 1, 0};
    light.enabled = false;

    sun.direction = glm::vec3{1, -0.5f, 0.5f};
    sun.enabled = true;

    app.plainColorShader = ShaderProgram{"shaders/plain_color.glsl", SHOW_LOGS};
    app.quad = Model{"res/models/quad.obj"};
    app.cube = Model{"res/models/cube.obj"};

//   ==================================================================

    app.loadModel  ("res/models/cube.obj",                          {  FLIP_TEXTURES,  FLIP_WINING_ORDER });
    app.loadModel  ("res/models/sphere/scene.gltf",                 {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadModel  ("res/models/lemon/lemon_4k.gltf",               {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadModel  ("res/models/apple/food_apple_01_4k.gltf",       {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadModel  ("res/models/backpack/backpack.obj",             { !FLIP_TEXTURES, !FLIP_WINING_ORDER });

    Texture tileTexture("res/textures/tile.png",           FLIP_TEXTURES, SRGB, GL_REPEAT);
    Texture concreteTexture("res/textures/concrete.jpg",   FLIP_TEXTURES, SRGB, GL_REPEAT);
    Texture oakTexture("res/textures/oak.jpg",             FLIP_TEXTURES, SRGB, GL_REPEAT);
    // app.loadTexture("res/textures/white.png",                       {  FLIP_TEXTURES });
    // app.loadTexture("res/textures/brick_wall.jpg",                  {  FLIP_TEXTURES });

// =========================== //

    app.currentModelIndex = 0;    // cube
    app.currentShaderIndex = 1;   // lighting

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

    const unsigned SHADOW_RESOLUTION = 1024;
    Texture depthMap;
    depthMap.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    Framebuffer depthMapFBO;
    depthMapFBO.bind();
    depthMapFBO.attach(depthMap, GL_DEPTH_ATTACHMENT);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    depthMapFBO.unbind();
    assert(depthMapFBO.isComplete());

// =========================== //

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        camera.update(app.deltatime);
        flashlight.position  = camera.position;
        flashlight.direction = camera.getFront();
        int lastWidth = camera.width, lastHeight = camera.height;
        glfwGetWindowSize(window, &camera.width, &camera.height);

        if(app.faceCulling) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);

// ============================ //
//     draw to the depth map    //
// ============================ //

        depthMapFBO.bind();
        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        renderer.clear();
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
        glm::mat4 lightView = glm::lookAt(-sun.direction, {0, 0, 0}, {0, 1, 0});

        app.shaders[6].bind();
        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.models[app.currentModelIndex].m_position);
        app.models[app.currentModelIndex].rotate(app.models[app.currentModelIndex].m_rotation);
        app.models[app.currentModelIndex].scale(app.models[app.currentModelIndex].m_scale);

        renderer.setMatrixUniforms(app.shaders[6], app.models[app.currentModelIndex].getModelMat(), lightView, lightProjection);
        renderer.drawb(app.models[app.currentModelIndex], app.shaders[6]); 

        glDisable(GL_CULL_FACE);
        renderer.setMatrixUniforms(app.shaders[6], glm::mat4{1}, lightView, lightProjection);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_CULL_FACE);


// ===================== //
//     draw the scene    //
// ===================== //

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear(app.clearColor);

        // draw the model
        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.models[app.currentModelIndex].m_position);
        app.models[app.currentModelIndex].rotate(app.models[app.currentModelIndex].m_rotation);
        app.models[app.currentModelIndex].scale(app.models[app.currentModelIndex].m_scale);

        glUniform1f(currentShader.getUniform("u_timepoint"), glfwGetTime());
        oakTexture.bind();
        renderer.draw(app.models[app.currentModelIndex], currentShader, camera); 

        glDisable(GL_CULL_FACE);
        currentShader.bind();
        renderer.setLightingUniforms(currentShader);
        renderer.setMaterialUniforms(currentShader, 0);
        renderer.setMatrixUniforms  (currentShader, glm::mat4{1}, camera);
        oakTexture.bind(0);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_CULL_FACE);

        if(light.enabled) {
            // draw the light cube
            app.cube.resetMatrix();
            app.cube.translate(light.position);
            app.cube.scale(glm::vec3{0.03125});
            app.plainColorShader.bind();
            glUniform3fv(app.plainColorShader.getUniform("u_color"), 1, &light.color.x);
            renderer.setMatrixUniforms(app.plainColorShader, app.cube.getModelMat(), camera);
            renderer.drawb(app.cube, app.plainColorShader);
        } 

// ================================= //
//     debug: visualise depth map    //
// ================================= //
        
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // renderer.clear(app.clearColor);
        // renderer.clear(app.clearColor);
        // app.shaders[5].bind();
        // glUniform1i(app.shaders[5].getUniform("u_texture"), 0);
        // depthMap.bind(0);
        // renderer.drawb(oneSideQuad, app.shaders[5]); 

// ================== //

        imguistuff(app, camera, light, flashlight, sun);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
}
