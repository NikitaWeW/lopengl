/*
i use this (gcc + ninja)
cmake -S . -B build -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS='-fdiagnostics-color=always -Wall' -G Ninja
cmake --build build && build/main --fast
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
#define currentShader app.shaders[app.displayShaders[app.currentShaderIndex]]

void imguistuff(Application &app, ControllableCamera &cam, PointLight &light, SpotLight &flashlight);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main(int argc, char **argv)
{
    bool fastLoad = false;
    for(int i = 0; i < argc; ++i) if(strcmp(argv[i], "--fast") == 0) fastLoad = true;
    printf("loading...\n"); // TODO: cool progress bar
    Application app;
    GLFWwindow *window = app.window;
    Model lightCube("res/models/cube.obj");
    Model oneSideQuad{"res/models/one_side_quad.obj"};
    ControllableCamera camera(window, {0, 0, 7}, {-90, 0, 0});
    PointLight light;
    SpotLight flashlight;
    Renderer renderer;
    Cubemap skybox("res/textures/skybox1", {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg"});
    
//  =========================================== 

    glfwGetWindowSize(window, &camera.width, &camera.height);

    renderer.getLights().push_back(&flashlight);
    renderer.getLights().push_back(&light);

    app.shaders = {
        {"shaders/basic.glsl",          SHOW_LOGS},
        {"shaders/lighting.glsl",       SHOW_LOGS},
        {"shaders/reflection.glsl",     SHOW_LOGS},
        {"shaders/refraction.glsl",     SHOW_LOGS},
        {"shaders/post_process.glsl",   SHOW_LOGS},
        {"shaders/skybox.glsl",         SHOW_LOGS},
        {"shaders/explode.glsl",        SHOW_LOGS},
        {"shaders/normals.glsl",        SHOW_LOGS},
        {"shaders/instancing.glsl",     SHOW_LOGS}
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 
    app.displayShaders = {0, 1, 2, 3, 6}; // shows in shader list.

    flashlight.position  = camera.position;
    flashlight.direction = camera.getFront();

    light.position= glm::vec3{2, 1, 3};
    light.enabled = false;

    app.plainColorShader = ShaderProgram{"shaders/plain_color.glsl", SHOW_LOGS};
    app.quad = Model{"res/models/quad.obj"};
    app.cube = Model{"res/models/cube.obj"};

    Framebuffer framebuffer;
    Texture cameraTexture{camera.width, camera.height, GL_CLAMP_TO_EDGE}; // will be set to window size
    Renderbuffer rb{GL_DEPTH24_STENCIL8, camera.width, camera.height}; 
    framebuffer.attachTexture(cameraTexture);
    framebuffer.attachRenderbuffer(rb);
    assert(framebuffer.isComplete());

    Framebuffer MSframebuffer;
    MultisampleTexture MScameraTexture{camera.width, camera.height, GL_CLAMP_TO_EDGE}; // will be set to window size
    MultisampleRenderbuffer MSrb{GL_DEPTH24_STENCIL8, camera.width, camera.height}; 
    MSframebuffer.attachTexture(MScameraTexture);
    MSframebuffer.attachRenderbuffer(MSrb);
    assert(MSframebuffer.isComplete());

//   ==================================================================

    app.loadModel  ("res/models/cube.obj",                          {  FLIP_TEXTURES,  FLIP_WINING_ORDER });
    app.loadModel  ("res/models/sphere/scene.gltf",                 {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadModel  ("res/models/lemon/lemon_4k.gltf",               {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadModel  ("res/models/apple/food_apple_01_4k.gltf",       {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadTexture("res/textures/tile.png",                        {  FLIP_TEXTURES });
    app.loadTexture("res/textures/white.png",                       {  FLIP_TEXTURES });
    app.loadTexture("res/textures/concrete.jpg",                    {  FLIP_TEXTURES });
if(!fastLoad) {
    app.loadModel  ("res/models/backpack/backpack.obj",             { !FLIP_TEXTURES, !FLIP_WINING_ORDER });
    // app.loadModel  ("res/models/rock/namaqualand_cliff_02_4k.gltf", {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    // app.loadModel  ("res/models/sponza/sponza.obj",                 {  FLIP_TEXTURES, !FLIP_WINING_ORDER });
    app.loadTexture("res/textures/oak.jpg",                         {  FLIP_TEXTURES });
    app.loadTexture("res/textures/brick_wall.jpg",                  {  FLIP_TEXTURES });
}

// =========================== //

    ShaderProgram &postProcessShader = app.shaders[4];
    ShaderProgram &skyboxShader      = app.shaders[5];
    ShaderProgram &normalShader      = app.shaders[7];

    app.currentTextureIndex = 2;  // concrete
    app.currentModelIndex = 0;    // cube
    app.currentShaderIndex = 0;   // basic

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

// =========================== //
//      render the scene       //
// =========================== //

        MSframebuffer.bind();
        renderer.clear(app.clearColor);

        // draw the model
        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.models[app.currentModelIndex].m_position);
        app.models[app.currentModelIndex].rotate(app.models[app.currentModelIndex].m_rotation);
        app.models[app.currentModelIndex].scale(app.models[app.currentModelIndex].m_scale);

        glUniform1f(currentShader.getUniform("u_timepoint"), glfwGetTime());
        app.textures[app.currentTextureIndex].bind();
        renderer.drawLighting(app.models[app.currentModelIndex], currentShader, camera); 

// ======================================================= //
//      render the plane that covers the entire window     //
// ======================================================= //

        framebuffer.unbind();

        renderer.clear(app.clearColor);
        postProcessShader.bind();
        glUniform1i(postProcessShader.getUniform("u_texture"), 0);
        cameraTexture.bind(0);
        renderer.draw(oneSideQuad, postProcessShader); 
        imguistuff(app, camera, light, flashlight);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, MSframebuffer.getRenderID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,   framebuffer.getRenderID());
        glBlitFramebuffer(0, 0, camera.width, camera.height, 0, 0, camera.width, camera.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        
// ================== //
//      end frame     //
// ================== //

        if(lastWidth != camera.width || lastHeight != camera.height) {
            // resize texture and renderbuffer according to window size
            cameraTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            rb.bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width, camera.height);

            MScameraTexture.bind();
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, camera.width, camera.height, GL_TRUE);
            MSrb.bind();
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, camera.width, camera.height);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++app.frameCounter;
        app.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ControllableCamera &camera = *static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) camera.locked = !camera.locked;
    if (camera.locked) {
        camera.firstCursorMove = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if(key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
        // evaluate fov
        if (camera.fov < 1.0f)
            camera.fov = 1.0f;
        if (camera.fov > 45.0f)
            camera.fov = 45.0f;
    } else {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    ControllableCamera *cam = static_cast<ControllableCamera *>(glfwGetWindowUserPointer(window));
    if(cam->locked) {
        cam->fov -= (float)yoffset * 4.5f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            if (cam->fov < 0.01f)
                cam->fov = 0.01f;
            if (cam->fov > 60.0f)
                cam->fov = 60.0f;
        } else {
            if (cam->fov < 1.0f)
                cam->fov = 1.0f;
            if (cam->fov > 45.0f)
                cam->fov = 45.0f;
        }
    } else {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    }
}