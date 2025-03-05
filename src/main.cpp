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
    }; // on shader reload contents will be recompiled, if fails failed shader will be restored. 

    app.cube = Model{"res/models/cube.obj", !FLIP_TEXTURES, FLIP_WINING_ORDER};
    app.camera = &camera;

    Model quad{"res/models/quad.obj"};

//   ==================================================================
    app.models = {
        app.cube,                                                           // 0
        {"res/models/sphere.obj",                          FLIP_TEXTURES }, // 1
        {"res/models/lemon/lemon_4k.gltf",                 FLIP_TEXTURES }, // 2
        {"res/models/wall/wall.obj",                       FLIP_TEXTURES }, // 3
        // {"res/models/backpack/backpack.obj",              !FLIP_TEXTURES }, // 4
    };
    { // do model specific stuff
        // how to get segfault 101
        auto normalmap = "res/textures/normalMaps/rough_normal.jpg";
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/wall/wall.obj"; })->getMeshes()[0].textures.push_back({"res/models/wall/height.jpg", !FLIP_TEXTURES, !SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "height"});
        std::find_if(app.models.begin(), app.models.end(), [](Model const &model){ return model.getFilepath() == "res/models/wall/wall.obj"; })->getMeshes()[0].textures.push_back({"res/models/wall/height.jpg", !FLIP_TEXTURES, !SRGB, GL_CLAMP_TO_EDGE, GL_LINEAR, "height"});
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
    // glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_CURSOR, camera.locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

// =========================== //
//  generate the SSAO kernel
// =========================== //
    constexpr size_t numSamples = 64;
    glm::vec3 SSAOkernel[numSamples];
    for(size_t i = 0; i < numSamples; ++i) {
        glm::vec3 sample{
            randRange(0.0f, 1.0f) * 2.0 - 1.0,
            randRange(0.0f, 1.0f) * 2.0 - 1.0,
            randRange(0.0f, 1.0f)
        };
        sample = glm::normalize(sample);
        sample *= randRange(0.0f, 1.0f);
        float scale = (float) i / numSamples;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        SSAOkernel[i] = sample;
    }

// =========================== //
//  generate the framebuffers
// =========================== //

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
//  generate scene
// =========================== //
    LOG_DEBUG("generating scene...");

//  ----------------------------------
    // input here
    constexpr unsigned numLights = 50;
    constexpr unsigned numModels = 1000;
    constexpr float radius = 50;
    Model sceneModel = app.models[0];
//  ----------------------------------

    glm::mat4 modelMatrices[numModels];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    for(unsigned i = 0; i < numModels; ++i) {
        glm::mat4 model{1.0f};
        model = glm::translate(model, { randRange(-radius, radius), randRange(-radius, radius), randRange(-radius, radius) });
        model = glm::scale(model, glm::vec3{ randRange(0.5f, 1.5f) });
        model = glm::rotate(model, 1.0f, {randRange(0.0f, 360.0f), randRange(0.0f, 360.0f), randRange(0.0f, 360.0f)});
        modelMatrices[i] = model;
    }

    VertexBuffer modelMatricesVB{modelMatrices, numModels * sizeof(glm::mat4)};
    InstancedArrayLayout modelMatricesVBlayout{
        {4, GL_FLOAT, 1},
        {4, GL_FLOAT, 1},
        {4, GL_FLOAT, 1},
        {4, GL_FLOAT, 1}
    };

    for(Mesh &mesh : sceneModel.getMeshes()) {
        mesh.va.addBuffer(modelMatricesVB, modelMatricesVBlayout);
    }

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
        }

// ===================== //
//     draw the scene    //
// ===================== //

        Gbuffer.bind();
        glFrontFace(GL_CCW);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear(app.clearColor);

        app.shaders[2].bind();
        // set uniforms
        glUniformMatrix4fv(app.shaders[2].getUniform("u_viewMat"),      1, GL_FALSE, &camera.getViewMatrix()[0][0]);
        glUniformMatrix4fv(app.shaders[2].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);

// ================== //

        sceneModel.resetMatrix();
        sceneModel.translate(app.currentModelPosition);
        sceneModel.rotate(app.currentModelRotation);
        sceneModel.scale(app.currentModelScale);
        glUniformMatrix4fv(app.shaders[2].getUniform("u_modelMat"), 1, GL_FALSE, &sceneModel.getModelMat()[0][0]);
        glUniformMatrix4fv(app.shaders[2].getUniform("u_normalMat"), 1, GL_FALSE, &glm::transpose(glm::inverse(sceneModel.getModelMat()))[0][0]);
        for(Mesh const &mesh : sceneModel.getMeshes()) {
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
         
            
            mesh.va.bind();
            mesh.ib.bind();
            glDrawElementsInstanced(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr, numModels);
        }


// ====================== //
//  draw the framebuffer
// ====================== //
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, camera.width, camera.height);
        renderer.clear();
        quad.resetMatrix();
        app.shaders[1].bind();

        renderer.setLightingUniforms(app.shaders[1]);
        glUniform3fv(app.shaders[1].getUniform("u_viewPos"), 1, &camera.position.x);
        glUniform1i(app.shaders[1].getUniform("u_material.position"), 0);
        glUniform1i(app.shaders[1].getUniform("u_material.normal"), 1);
        glUniform1i(app.shaders[1].getUniform("u_material.albedoSpecular"), 2);
        glUniform1i(app.shaders[1].getUniform("u_texture"), 2);
        GbufferPositionTexture.bind(0);
        GbufferNormalTexture.bind(1);
        GbufferAlbedoSpecularTexture.bind(2);
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
                app.models[1].resetMatrix();
                app.models[1].translate(plight->position);
                app.models[1].scale(glm::vec3{plight->getRadius()});
                app.shaders[3].bind();
                glUniform3fv(app.shaders[3].getUniform("u_color"), 1, &plight->color.x);
                glUniformMatrix4fv(app.shaders[3].getUniform("u_modelMat"), 1, GL_FALSE, &app.models[1].getModelMat()[0][0]);
                glUniformMatrix4fv(app.shaders[3].getUniform("u_viewMat"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
                glUniformMatrix4fv(app.shaders[3].getUniform("u_projectionMat"),1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
                renderer.draw(app.models[1]);
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