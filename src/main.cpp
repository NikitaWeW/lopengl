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
#include "opengl/Renderer.hpp"
#include "utils/ControllableCamera.hpp"
#include "opengl/Framebuffer.hpp"
#include "opengl/UniformBuffer.hpp"
#include "opengl/Cubemap.hpp"

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
#define SHOW_LOGS         true // for readability
#define LOAD_NOW          true
#define FLIP_TEXTURES     true
#define FLIP_WINING_ORDER true
#define SRGB              true
#define currentShader app.shaders[app.displayShaders[app.currentShaderIndex]]

void imguistuff(Application &app, ControllableCamera &cam);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
float lerp(float a, float b, float x) { return a + x * (b - a); }

int main(int argc, char **argv)
{
    printf("loading...\n"); // TODO: cool progress bar
    Application app;
    GLFWwindow *window = app.window;
    ControllableCamera camera(window, {0, 0, 3}, {-90, 0, 0});
    camera.far = 500;
    Renderer renderer;
    Cubemap skybox("res/textures/skybox1", {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg"});
    
//  =========================================== 

    // glfwGetWindowSize(window, &camera.width, &camera.height);

    app.shaders = {
        {"shaders/basic.glsl",              SHOW_LOGS}, // 0
        {"shaders/defferred_lighting.glsl", SHOW_LOGS}, // 1
        {"shaders/defferred.glsl",          SHOW_LOGS}, // 2
        {"shaders/plain_color.glsl",        SHOW_LOGS}, // 3
        {"shaders/ssao.glsl",               SHOW_LOGS}, // 4
        {"shaders/blur.glsl",               SHOW_LOGS}, // 5
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 

    app.cube = Model{"res/models/cube.obj", !FLIP_TEXTURES, FLIP_WINING_ORDER};
    app.sphere = Model{"res/models/sphere.obj", FLIP_TEXTURES };
    app.camera = &camera;

    Model quad{"res/models/quad.obj"};

//   ==================================================================
    app.models = {
        app.cube,                                              // 0
        app.sphere,                                            // 1
        {"res/models/backpack/backpack.obj", !FLIP_TEXTURES }, // 2
    };
    { // do model specific stuff
        // how to get segfault 101
        auto normalmap = "res/textures/normalMaps/rough_normal.jpg";
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/cube.obj"; })->getMeshes()[0].textures.push_back({"res/textures/concrete.jpg", !FLIP_TEXTURES, SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "diffuse"});
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/cube.obj"; })->getMeshes()[0].textures.push_back({normalmap, !FLIP_TEXTURES, !SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "normal"});
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/sphere.obj"; })->getMeshes()[0].textures.push_back({"res/textures/concrete.jpg", !FLIP_TEXTURES, SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "diffuse"});
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/sphere.obj"; })->getMeshes()[0].textures.push_back({normalmap, !FLIP_TEXTURES, !SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "normal"});
    }

// =========================== //
    // how to get segfault 101 pt. 2
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

// =========================== //
//  generate the SSAO stuff
// =========================== //
    constexpr size_t SSAOnumSamples = 64;
    constexpr size_t SSAOnoiseSide = 16;
//   ----------------------------------
    glm::vec3 SSAOkernel[SSAOnumSamples];
    for(size_t i = 0; i < SSAOnumSamples; ++i) {
        glm::vec3 sample{
            randRange(0.0f, 1.0f) * 2.0 - 1.0,
            randRange(0.0f, 1.0f) * 2.0 - 1.0,
            randRange(0.0f, 1.0f)
        };
        sample = glm::normalize(sample);
        sample *= randRange(0.0f, 1.0f);
        float scale = (float) i / SSAOnumSamples;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        SSAOkernel[i] = sample;
    }
    glm::vec3 SSAOnoise[SSAOnoiseSide*SSAOnoiseSide];
    for(size_t i = 0; i < SSAOnoiseSide*SSAOnoiseSide; ++i) {
        glm::vec3 noise{
            randRange(-1.0f, 1.0f),
            randRange(-1.0f, 1.0f),
            0
        };
        SSAOnoise[i] = noise;
    }
    Texture SSAOnoiseTexture;
    SSAOnoiseTexture.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAOnoiseSide, SSAOnoiseSide, 0, GL_RGB, GL_FLOAT, nullptr);
    { // seems unnecessary, but whatever
        Texture SSAOnoBlurNoiseTexture;
        SSAOnoiseTexture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SSAOnoiseSide, SSAOnoiseSide, 0, GL_RGB, GL_FLOAT, SSAOnoise);

        Framebuffer SSAOnoiseFBO;
        SSAOnoiseFBO.bind();
        SSAOnoiseFBO.attach(SSAOnoiseTexture, GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, SSAOnoiseSide, SSAOnoiseSide);
        app.shaders[5].bind();
        glUniform1i(app.shaders[5].getUniform("u_texture"), 0);
        SSAOnoBlurNoiseTexture.bind();
        renderer.draw(quad);
    }

// =========================== //
//  generate the framebuffers
// =========================== //
    Framebuffer SSAOfbo;
    SSAOfbo.bind();
    Texture SSAOtexture{1, 1, GL_RED, GL_CLAMP_TO_EDGE, GL_NEAREST};
    SSAOfbo.attach(SSAOtexture, GL_COLOR_ATTACHMENT0);
    Renderbuffer SSAOrbo{GL_DEPTH24_STENCIL8, 1, 1};
    SSAOfbo.attach(SSAOrbo, GL_DEPTH_STENCIL_ATTACHMENT);

    Framebuffer Gbuffer;
    Gbuffer.bind();

    Texture GbufferPositionTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferPositionTexture, GL_COLOR_ATTACHMENT0);
    
    Texture GbufferNormalTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferNormalTexture, GL_COLOR_ATTACHMENT1);
    
    Texture GbufferAlbedoSpecularTexture{1, 1, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR};
    Gbuffer.attach(GbufferAlbedoSpecularTexture, GL_COLOR_ATTACHMENT2);

    Renderbuffer GbufferRBO{GL_DEPTH24_STENCIL8, 1, 1};
    Gbuffer.attach(GbufferRBO, GL_DEPTH_STENCIL_ATTACHMENT);


    unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(sizeof(attachments) / sizeof(*attachments), attachments);

    assert(Gbuffer.isComplete());
    Gbuffer.unbind();

// =========================== //
//  generate the lights
// =========================== //
//  ----------------------------------
    // input here
    constexpr unsigned numLights = 4;
    constexpr float radius = 5;
//  ----------------------------------

    PointLight lights[numLights];
    for(unsigned i = 0; i < numLights; ++i) {
        PointLight light;
        light.position = { randRange(-radius, radius), randRange(-radius, radius), randRange(-radius, radius) };
        light.color = { randRange(0.0f, 2.0f), randRange(0.0f, 2.0f), randRange(0.0f, 2.0f) };
        light.attenuation = randRange(0.07f, 0.09f);
        lights[i] = light; // keep them in the memory
        renderer.getLights().push_back(lights + i);
    }

// =========================== //

    LOG_INFO("loaded!");

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        int prevWidth = camera.width, prevHeight = camera.height;
        camera.update(app.deltatime);
        glfwGetWindowSize(window, &camera.width, &camera.height);

        if(prevWidth != camera.width || prevHeight != camera.height) { // close your eyes here
            GbufferPositionTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferNormalTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferAlbedoSpecularTexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            GbufferRBO.bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width, camera.height);
            assert(Gbuffer.isComplete());
            
            SSAOtexture.bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camera.width, camera.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            SSAOrbo.bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width, camera.height);
            assert(SSAOfbo.isComplete());
        }

// ================== //
//  geometry pass
// ================== //

        Gbuffer.bind();
        glFrontFace(GL_CCW);
        glViewport(0, 0, camera.width, camera.height);
        glClearColor(app.clearColor.r, app.clearColor.g, app.clearColor.b, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        app.shaders[2].bind();
        // set uniforms
        glUniformMatrix4fv(app.shaders[2].getUniform("u_viewMat"),      1, GL_FALSE, &camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[2].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);

        app.models[2].resetMatrix();
        app.models[2].translate(app.currentModelPosition);
        app.models[2].rotate(app.currentModelRotation);
        app.models[2].scale(app.currentModelScale);
        glUniformMatrix4fv(app.shaders[2].getUniform("u_modelMat"), 1, GL_FALSE, &app.models[2].getModelMat()[0][0]);
        glUniformMatrix4fv(app.shaders[2].getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(app.models[2].getModelMat()))[0][0]);
        for(Mesh const &mesh : app.models[2].getMeshes()) {
            bool specularSet = false;
            bool normalSet = false;
            bool heightSet = false;
            unsigned int textureCount = 0;
            for(Texture const &texture : mesh.textures) {
                int location = app.shaders[2].getUniform("u_material." + texture.type);
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

            glUniform1i(app.shaders[2].getUniform("u_material.specularSet"), specularSet);
            glUniform1i(app.shaders[2].getUniform("u_material.normalSet"), normalSet);
            glUniform1i(app.shaders[2].getUniform("u_material.heightSet"), heightSet);
         
            renderer.draw(mesh);
        }

// ====================== //
//  SSAO pass
// ====================== //
        SSAOfbo.bind();
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear();
        app.shaders[4].bind();
        
        glUniform1i(app.shaders[4].getUniform("u_material.position"), 0); GbufferPositionTexture.bind(0);
        glUniform1i(app.shaders[4].getUniform("u_material.normal"), 1);   GbufferNormalTexture.bind(1);
        glUniform1i(app.shaders[4].getUniform("u_material.noise"), 2);    SSAOnoiseTexture.bind(2);
        glUniform3fv(app.shaders[4].getUniform("u_samples"), SSAOnumSamples, &SSAOkernel->x);
        glUniform2f(app.shaders[4].getUniform("u_noiseScale"), (GLfloat) camera.width / SSAOnoiseSide, (GLfloat) camera.height / SSAOnoiseSide);
        glUniformMatrix4fv(app.shaders[4].getUniform("u_viewMat"),      1, GL_FALSE, &camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[4].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);

        quad.resetMatrix();
        renderer.draw(quad);

// ====================== //
//  backbuffer pass
// ====================== //
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear();
        quad.resetMatrix();
        app.shaders[1].bind();

        renderer.setLightingUniforms(app.shaders[1]);
        glUniform3fv(app.shaders[1].getUniform("u_viewPos"), 1, &camera.position.x);
        glUniform1i(app.shaders[1].getUniform("u_material.position"), 0);       GbufferPositionTexture.bind(0);
        glUniform1i(app.shaders[1].getUniform("u_material.normal"), 1);         GbufferNormalTexture.bind(1);
        glUniform1i(app.shaders[1].getUniform("u_material.albedoSpecular"), 2); GbufferAlbedoSpecularTexture.bind(2);
        glUniform1i(app.shaders[1].getUniform("u_material.ssao"), 3);           SSAOtexture.bind(3);
        
        glUniform1i(app.shaders[1].getUniform("u_texture"), 2);

        glDepthMask(GL_FALSE);
        renderer.draw(quad);
        glDepthMask(GL_TRUE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, Gbuffer.getRenderID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        glBlitFramebuffer(0, 0, camera.width, camera.height, 0, 0, camera.width, camera.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        
        for(Light const *light : renderer.getLights()) {
            if(light->enabled || light->type == POINT) {
                // draw the light cube
                PointLight const *plight = dynamic_cast<PointLight const *>(light);
                assert(plight);
                app.cube.resetMatrix();
                app.cube.translate(plight->position);
                app.cube.scale(glm::vec3{0.05f});
                app.shaders[3].bind();
                glUniform3fv(app.shaders[3].getUniform("u_color"), 1, &plight->color.x);
                glUniformMatrix4fv(app.shaders[3].getUniform("u_modelMat"), 1, GL_FALSE, &app.cube.getModelMat()[0][0]);
                glUniformMatrix4fv(app.shaders[3].getUniform("u_viewMat"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
                glUniformMatrix4fv(app.shaders[3].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
                renderer.draw(app.cube);
            } 
        }

// ================== //


        imguistuff(app, camera); // bad

        if(app.frameCounter % 100 == 0) glfwSetWindowTitle(window, ("lopengl -- " + std::to_string((int) glm::round(1 / app.deltatime)) + " FPS").c_str());
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