#include "glad/gl.h"
#include "opengl/Renderer.hpp"
#include "imgui.h"
#include "Application.hpp"

void renderthing(Application &app, Renderer &renderer, Camera &camera) {
    renderer.clear(app.clearColor);
    app.models[app.currentModelIndex].resetMatrix();
    app.models[app.currentModelIndex].translate(app.models[app.currentModelIndex].m_position);
    app.models[app.currentModelIndex].rotate(app.models[app.currentModelIndex].m_rotation);
    app.models[app.currentModelIndex].scale(app.models[app.currentModelIndex].m_scale);

    app.textures[app.currentTextureIndex].bind();
    renderer.draw(app.models[app.currentModelIndex], app.shaders[app.currentShaderIndex], camera); 
    app.textures[app.currentTextureIndex].unbind();
}