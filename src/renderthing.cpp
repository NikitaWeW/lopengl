#include "glad/gl.h"
#include "opengl/Renderer.hpp"
#include "imgui.h"
#include "Application.hpp"

void renderthing(Application &app, Renderer &renderer, Camera &camera) {
    renderer.clear(app.clearColor);
    app.currentModel->resetMatrix();
    app.currentModel->translate(app.currentModel->m_position);
    app.currentModel->rotate(app.currentModel->m_rotation);
    app.currentModel->scale(app.currentModel->m_scale);

    if(app.currentTexture) app.currentTexture->bind();
    renderer.draw(*app.currentModel, app.shaders[app.currentShaderIndex], camera); 
    if(app.currentTexture) app.currentTexture->unbind();
}