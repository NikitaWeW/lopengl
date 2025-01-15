#include "glad/gl.h"
#include "Renderer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "logger.h"
#include <cassert>

Renderer::Renderer() {
};

void Renderer::clear(glm::vec3 clearColor) const
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::setLightingUniforms(ShaderProgram const &shader) const
{
    unsigned pointLightCount= 0;
    unsigned dirLightCount  = 0;
    unsigned spotLightCount = 0;
    for(unsigned i = 0; i < m_lights.size(); ++i) {
        if(m_lights[i]->enabled) m_lights[i]->setUniforms(shader, pointLightCount, dirLightCount, spotLightCount);
    }
    glUniform1i(shader.getUniform("u_pointLightCount"), pointLightCount);
    glUniform1i(shader.getUniform("u_dirLightCount"),   dirLightCount);
    glUniform1i(shader.getUniform("u_spotLightCount"),  spotLightCount);
}

void Renderer::setMaterialUniforms(ShaderProgram const &shader, Mesh const &mesh) const
{
    bool specularSet = false;
    if(mesh.textures.size() == 0) { // use texture at slot 0 as diffuse texture
        glUniform1i(shader.getUniform("u_material.diffuse"), 0);
    }
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
}

void Renderer::setMaterialUniforms(ShaderProgram const &shader, int const diffuse, int const specular) const
{
    glUniform1i(shader.getUniform("u_material.diffuse"), diffuse);
    if(specular >= 0) glUniform1i(shader.getUniform("u_material.specular"), specular);
    glUniform1f(shader.getUniform("u_material.shininess"), 32);
    glUniform1i(shader.getUniform("u_specularSet"), specular >= 0);
}

void Renderer::setMatrixUniforms(ShaderProgram const &shader, glm::mat4 const &modelMatrix, Camera const &camera) const
{
    setMatrixUniforms(shader, modelMatrix, camera.getViewMatrix(),camera.getProjectionMatrix());
}

void Renderer::setMatrixUniforms(ShaderProgram const &shader, glm::mat4 const &modelMatrix, glm::mat4 const &viewMatrix, glm::mat4 const &projectionMatrix) const
{
    if(shader.getUniform("u_normalMat") != -1) {
        glm::mat4 normalMat = glm::transpose(glm::inverse(modelMatrix));
        glUniformMatrix4fv(shader.getUniform("u_normalMat"), 1, GL_FALSE, &normalMat[0][0]);
    }
    glUniformMatrix4fv(shader.getUniform("u_modelMat"),     1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_viewMat"),      1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(shader.getUniform("u_projectionMat"),1, GL_FALSE, &projectionMatrix[0][0]);
}

void Renderer::draw(Mesh const &mesh, ShaderProgram const &shader) const
{
    shader.bind();
    mesh.va.bind();
    mesh.ib.bind();
    glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::draw(Model const &model, ShaderProgram const &shader, Camera const &camera) const
{
    shader.bind();
    for(Mesh const &mesh : model.getMeshes()) {
        setLightingUniforms(shader);
        setMaterialUniforms(shader, mesh);
        setMatrixUniforms(shader, model.getModelMat(), camera);
        glUniform3fv(shader.getUniform("u_viewPos"), 1, &camera.position.x);
        draw(mesh, shader);
    }
}

void Renderer::drawb(Model const &model, ShaderProgram const &shader) const
{
    shader.bind();
    for(Mesh const &mesh : model.getMeshes()) {
        draw(mesh, shader);
    }
}
