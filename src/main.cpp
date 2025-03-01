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

void imguistuff(Application &app, ControllableCamera &cam);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

int main(int argc, char **argv)
{
    printf("loading...\n"); // TODO: cool progress bar
    Application app;
    GLFWwindow *window = app.window;
    ControllableCamera camera(window, {0, 0, 3}, {-90, 0, 0});
    Renderer renderer;
    Cubemap skybox("res/textures/skybox1", {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg"});
    
//  =========================================== 

    // glfwGetWindowSize(window, &camera.width, &camera.height);

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
        {"shaders/skybox.glsl",SHOW_LOGS}, // 9
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 
    app.displayShaders = {0, 1, 2, 3, 4}; // shows in shader list.

    app.cube = Model{"res/models/cube.obj", !FLIP_TEXTURES, FLIP_WINING_ORDER};
    app.camera = &camera;

    Model quad{"res/models/quad.obj"};

//   ==================================================================
    app.models = {
        app.cube,
        {"res/models/sphere/scene.gltf",                   FLIP_TEXTURES },
        {"res/models/lemon/lemon_4k.gltf",                 FLIP_TEXTURES },
        {"res/models/wall/wall.obj",                       FLIP_TEXTURES },
        {"res/models/backpack/backpack.obj",              !FLIP_TEXTURES },
    };
    { // do model specific stuff
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/wall/wall.obj"; })->getMeshes()[0].textures.push_back({"res/models/wall/height.jpg", !FLIP_TEXTURES, !SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "height"});
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/cube.obj"; })->getMeshes()[0].textures.push_back({"res/textures/oak.jpg", !FLIP_TEXTURES, SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "diffuse"});
    }

// =========================== //
    // how to get segfault 101
    app.currentModelIndex = 0;    // cube
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
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    LOG_INFO("loaded!");

// =========================== //

    Texture HDRtexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Renderbuffer HDRrbo{GL_DEPTH24_STENCIL8, 1, 1};
    Framebuffer HDRframebuffer;
    HDRframebuffer.bind();
    HDRframebuffer.attach(HDRtexture, GL_COLOR_ATTACHMENT0);
    HDRframebuffer.attach(HDRrbo, GL_DEPTH_STENCIL_ATTACHMENT);
    assert(HDRframebuffer.isComplete());
    HDRframebuffer.unbind();

// =========================== //

    Framebuffer Gbuffer;
    Gbuffer.bind();

    Texture GbufferPositionTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferPositionTexture, GL_COLOR_ATTACHMENT0);
    
    Texture GbufferNormalTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferNormalTexture, GL_COLOR_ATTACHMENT1);
    
    Texture GbufferAlbedoTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferAlbedoTexture, GL_COLOR_ATTACHMENT2);
    
    Texture GbufferAlbedoSpecularTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferAlbedoSpecularTexture, GL_COLOR_ATTACHMENT3);

    Renderbuffer GbufferRBO{GL_DEPTH24_STENCIL8, 1, 1};
    Gbuffer.attach(HDRrbo, GL_DEPTH_STENCIL_ATTACHMENT);


    unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(sizeof(attachments) / sizeof(*attachments), attachments);

    assert(Gbuffer.isComplete());
    Gbuffer.unbind();

// =========================== //

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        int prevWidth = camera.width, prevHeight = camera.height;
        camera.update(app.deltatime);
        glfwGetWindowSize(window, &camera.width, &camera.height);

        if(prevWidth != camera.width || prevHeight != camera.height) {
            HDRtexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            HDRrbo.bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width, camera.height);
            
            GbufferPositionTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferNormalTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferAlbedoTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferAlbedoSpecularTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferRBO.bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width, camera.height);
        }

// ===================== //
//     draw the scene    //
// ===================== //

        glEnable(GL_CULL_FACE);
        HDRframebuffer.bind();
        glFrontFace(GL_CCW);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear(app.clearColor);

        currentShader.bind();
        // set uniforms
        glUniform3fv(currentShader.getUniform("u_viewPos"), 1, &camera.position.x);
        glUniformMatrix4fv(currentShader.getUniform("u_viewMat"),      1, GL_FALSE, &camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
        renderer.setLightingUniforms(currentShader); // it sucks.

// ================== //

        app.models[app.currentModelIndex].resetMatrix();
        app.models[app.currentModelIndex].translate(app.currentModelPosition);
        app.models[app.currentModelIndex].rotate(app.currentModelRotation);
        app.models[app.currentModelIndex].scale(app.currentModelScale);
        glUniformMatrix4fv(currentShader.getUniform("u_modelMat"), 1, GL_FALSE, &app.models[app.currentModelIndex].getModelMat()[0][0]);
        glUniformMatrix4fv(currentShader.getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(app.models[app.currentModelIndex].getModelMat()))[0][0]);
        for(Mesh const &mesh : app.models[app.currentModelIndex].getMeshes()) {
            bool specularSet = false;
            bool normalSet = false;
            bool heightSet = false;
            unsigned int textureCount = 0;
            for(Texture const &texture : mesh.textures) {
                int location = currentShader.getUniform("u_material." + texture.type);
                if(location != -1) {
                    glUniform1i(location, textureCount);
                    texture.bind(textureCount);
                    ++textureCount;
                }
                if(texture.type == "specular") {
                    specularSet = true;
                } else if(texture.type == "normal") {
                    normalSet = true;
                } else if(texture.type == "height") {
                    heightSet = true;
                }
            }
            glUniform1f(currentShader.getUniform("u_material.shininess"), mesh.material.shininess);

            glUniform1i(currentShader.getUniform("u_material.specularSet"), specularSet);
            glUniform1i(currentShader.getUniform("u_material.normalSet"), normalSet);
            glUniform1i(currentShader.getUniform("u_material.heightSet"), heightSet);
            
            renderer.draw(mesh);
        }

// ================== //
        
        for(Light const *light : renderer.getLights()) {
            if(light->enabled || light->type == POINT) {
                // draw the light cube
                PointLight const *plight = dynamic_cast<PointLight const *>(light);
                assert(plight);
                app.cube.resetMatrix();
                app.cube.translate(plight->position);
                app.cube.scale(glm::vec3{0.03125});
                app.shaders[8].bind();
                glUniform3fv(app.shaders[8].getUniform("u_color"), 1, &plight->color.x);
                glUniformMatrix4fv(app.shaders[8].getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
                glUniformMatrix4fv(app.shaders[8].getUniform("u_viewMat"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
                glUniformMatrix4fv(app.shaders[8].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
                renderer.draw(app.cube);
            } 
        }

// ====================== //
//  draw the framebuffer
// ====================== //

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear();
        quad.resetMatrix();
        app.shaders[5].bind();
        glUniform1i(app.shaders[5].getUniform("u_texture"), 0);
        glUniform1f(app.shaders[5].getUniform("u_exposure"), app.exposure);
        HDRtexture.bind(0);
        renderer.draw(quad);

// ================== //


        imguistuff(app, camera);

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