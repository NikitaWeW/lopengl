#include "scenes.hpp"

void scenes::SingleModelScene::onRender(Application &app, Renderer &renderer, Camera &camera)
{
    renderer.clear(app.clearColor);
    app.currentModel->resetMatrix();
    app.currentModel->translate(app.currentModel->m_position);
    app.currentModel->rotate(app.currentModel->m_rotation);
    app.currentModel->scale(app.currentModel->m_scale);

    if(app.currentTexture) app.currentTexture->bind();
    renderer.draw(*app.currentModel, app.shaders[app.currentShaderIndex], camera); 
    if(app.currentTexture) app.currentTexture->unbind();
}

void scenes::SingleModelScene::onImGuiRender(Application &app)
{
    if(app.objectOutline) ImGui::ColorEdit3("outline color", &app.outlineColor.x);
    ImGui::Text("model");
    ImGui::DragFloat3("model position", &app.currentModel->m_position.x, 0.01f);
    ImGui::DragFloat3("rotation", &app.currentModel->m_rotation.x, 0.5f);
    ImGui::DragFloat3("scale", &app.currentModel->m_scale.x, 0.01f);
    if (ImGui::Button("reset model"))
    {
        app.currentModel->m_position = glm::vec3(0);
        app.currentModel->m_rotation = glm::vec3(0);
        app.currentModel->m_scale = glm::vec3(1);
    }
}
