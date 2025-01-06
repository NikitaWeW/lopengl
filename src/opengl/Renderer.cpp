#include "glad/gl.h"
#include "Renderer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "logger.h"
#include <cassert>

Renderer::Renderer() {
    matriciesUBO.bindingPoint(0);
};

void Renderer::clear(glm::vec3 clearColor) const
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::drawLighting(Model const &model, Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const
{
    shader.bind();
    // TODO: refactor

    unsigned pointLightCount= 0;
    unsigned dirLightCount  = 0;
    unsigned spotLightCount = 0;
    for(unsigned i = 0; i < m_lights.size(); ++i) {
        if(m_lights[i]->enabled) m_lights[i]->setUniforms(shader, pointLightCount, dirLightCount, spotLightCount);
    }
    glUniform1i(shader.getUniform("u_pointLightCount"), pointLightCount);
    glUniform1i(shader.getUniform("u_dirLightCount"),   dirLightCount);
    glUniform1i(shader.getUniform("u_spotLightCount"),  spotLightCount);
    glm::mat4 normalMat = glm::transpose(glm::inverse(model.getModelMat()));
    glUniformMatrix4fv(shader.getUniform("u_modelMat"),     1, GL_FALSE, &model.getModelMat()[0][0]);
    // glUniformMatrix4fv(shader.getUniform("u_viewMat"),      1, GL_FALSE, &viewMat[0][0]);
    // glUniformMatrix4fv(shader.getUniform("u_projectionMat"),1, GL_FALSE, &projectionMat[0][0]);
    // glUniformMatrix4fv(shader.getUniform("u_normalMat"),    1, GL_FALSE, &normalMat[0][0]);

    matriciesUBO.bind(); // dont do this
    glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(glm::mat4), sizeof(glm::mat4), &viewMat);
    glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(glm::mat4), sizeof(glm::mat4), &projectionMat);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), &normalMat);

    bool specularSet = false;
    for(Mesh const &mesh : model.getMeshes()) {
        mesh.va.bind();
        mesh.ib.bind();

        // process model textures
        std::vector<unsigned> texturesToUnbind; // wanna unbind textuers after render
        if(mesh.textures.size() == 0) { // use texture at slot 0 as diffuse texture
            glUniform1i(shader.getUniform("u_material.diffuse"), 0);
        }
        unsigned int textureCount = 1; // leave 0 for other purposes
        for(Texture const &texture : mesh.textures) {
            int location = shader.getUniform("u_material." + texture.type);
            if(location != -1) {
                glUniform1i(location, textureCount);
                texture.bind(textureCount);
                texturesToUnbind.push_back(textureCount);
                ++textureCount;
            }
            if(texture.type == "specular") {
                specularSet = true;
            }
        }
        if(!specularSet) {
            Texture::unbindStatic(textureCount);
            glUniform1i(shader.getUniform("u_material.specular"), textureCount);
        }

        glUniform1f(shader.getUniform("u_material.shininess"), mesh.material.shininess);
        glUniform1i(shader.getUniform("u_specularSet"), specularSet);
        glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        mesh.va.unbind();
        mesh.ib.unbind();
        for(unsigned slot : texturesToUnbind) {
            Texture::unbindStatic(slot);
        }
    }
    shader.unbind();
    glActiveTexture(GL_TEXTURE0);
}
void Renderer::drawLighting(Model const & model, Shader const & shader, Camera const & camera) const
{
    shader.bind();
    glUniform3fv(shader.getUniform("u_viewPos"), 1, &camera.position.x);
    drawLighting(model, shader, camera.getViewMatrix(), camera.getProjectionMatrix());
}

void Renderer::drawMaterial(Model const &model, Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const
{
    shader.bind();

    glm::mat4 normalMat = glm::transpose(glm::inverse(model.getModelMat()));
    glUniformMatrix4fv(shader.getUniform("u_modelMat"),     1, GL_FALSE, &model.getModelMat()[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_viewMat"),      1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_projectionMat"),1, GL_FALSE, &projectionMat[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_normalMat"),    1, GL_FALSE, &normalMat[0][0]);

    bool specularSet = false;
    for(Mesh const &mesh : model.getMeshes()) {
        mesh.va.bind();
        mesh.ib.bind();

        // process model textures
        if(mesh.textures.size() == 0) // use texture at slot 0 as diffuse texture
            glUniform1i(shader.getUniform("u_material.diffuse"), 0);
        unsigned int textureCount = 1; // leave 0 for other purposes
        for(Texture const &texture : mesh.textures) {
            int location = shader.getUniform("u_material." + texture.type);
            if(location != -1) {
                glUniform1i(location, textureCount);
                texture.bind(textureCount);
                ++textureCount;
            }
            if(texture.type == "specular") {
                specularSet = true;
            }
        }
        if(!specularSet) {
            Texture::unbindStatic(textureCount);
            glUniform1i(shader.getUniform("u_material.specular"), textureCount);
        }

        glUniform1f(shader.getUniform("u_material.shininess"), mesh.material.shininess);
        glUniform1i(shader.getUniform("u_specularSet"), specularSet);
        glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        mesh.va.unbind();
        mesh.ib.unbind();
    }
    shader.unbind();
    glActiveTexture(GL_TEXTURE0);
}

void Renderer::drawMaterial(Model const &model, Shader const &shader, Camera const &camera) const
{
    shader.bind();
    glUniform3fv(shader.getUniform("u_viewPos"), 1, &camera.position.x);
    drawMaterial(model, shader, camera.getViewMatrix(), camera.getProjectionMatrix());
}

void Renderer::draw(Model const &model, Shader const &shader) const
{
    shader.bind();
    for(Mesh const &mesh : model.getMeshes()) {
        mesh.va.bind();
        mesh.ib.bind();
        glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        mesh.va.unbind();
        mesh.ib.unbind();
    }
    shader.unbind();
}

void Renderer::draw(Model const &model, Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const
{
    shader.bind();
    glm::mat4 normalMat = glm::transpose(glm::inverse(model.getModelMat()));
    glUniformMatrix4fv(shader.getUniform("u_modelMat"),     1, GL_FALSE, &model.getModelMat()[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_viewMat"),      1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_projectionMat"),1, GL_FALSE, &projectionMat[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_normalMat"),    1, GL_FALSE, &normalMat[0][0]);
    for(Mesh const &mesh : model.getMeshes()) {
        mesh.va.bind();
        mesh.ib.bind();
        glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        mesh.va.unbind();
        mesh.ib.unbind();
    }
    shader.unbind();
}

void Renderer::draw(Model const &model, Shader const &shader, Camera const &camera) const
{
    shader.bind();
    glUniform3fv(shader.getUniform("u_viewPos"), 1, &camera.position.x);
    draw(model, shader, camera.getViewMatrix(), camera.getProjectionMatrix());
}