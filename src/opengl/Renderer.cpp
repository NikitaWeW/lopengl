#include "glad/gl.h"
#include "Renderer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "logger.h"
#include <cassert>

Renderer::Renderer() = default;

void Renderer::clear(glm::vec3 clearColor) const
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::draw(Model const &model, Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const
{
    shader.bind();

    unsigned pointLightCount= 0;
    unsigned dirLightCount  = 0;
    unsigned spotLightCount = 0;
    for(int i = 0; i < m_lights.size(); ++i) {
        if(m_lights[i]->enabled) m_lights[i]->setUniforms(shader, pointLightCount, dirLightCount, spotLightCount);
    }
    glUniform1i(shader.getUniform("u_pointLightCount"), pointLightCount);
    glUniform1i(shader.getUniform("u_dirLightCount"),   dirLightCount);
    glUniform1i(shader.getUniform("u_spotLightCount"),  spotLightCount);
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
        std::vector<unsigned> texturesToUnbind; // wanna unbind textuers after render
        if(mesh.textures.size() == 0) { 
            glUniform1i(shader.getUniform("u_material.diffuse"), 0);
            Texture::unbindStatic(1);
            glUniform1i(shader.getUniform("u_material.specular"), 1);
        }
        unsigned int textureCount = 1; // leave 0 for other purposes
        for(Texture const &texture : mesh.textures) {
            unsigned location = shader.getUniform("u_material." + texture.type);
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

        if(mesh.winding == GL_NONE) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glFrontFace(mesh.winding);
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

void Renderer::draw(Model const & model, Shader const & shader, Camera const & camera) const
{
    shader.bind();
    glUniform3fv(shader.getUniform("u_viewPos"), 1, &camera.position.x);
    draw(model, shader, camera.getViewMatrix(), camera.getProjectionMatrix());
}
