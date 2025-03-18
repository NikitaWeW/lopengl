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
    assert(false);
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

// void Renderer::setMaterialUniforms(ShaderProgram const &shader, Mesh const &mesh) const
// {
//     bool specularSet = false;
//     unsigned int textureCount = 1; // leave 0 for other purposes
//     for(Texture const &texture : mesh.textures) {
//         int location = shader.getUniform("u_material." + texture.type);
//         if(location != -1) {
//             glUniform1i(location, textureCount);
//             texture.bind(textureCount);
//             ++textureCount;
//         }
//         if(texture.type == "specular") {
//             specularSet = true;
//         }
//     }

//     glUniform1f(shader.getUniform("u_material.shininess"), mesh.material.shininess);
//     glUniform1i(shader.getUniform("u_specularSet"), specularSet);
// }


void Renderer::draw(Mesh const &mesh) const
{
    assert(false);
    // mesh.va.bind();
    // mesh.ib.bind();
    // glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
}


void Renderer::draw(Model const &model) const
{
    for(Mesh const &mesh : model.getMeshes()) {
        draw(mesh);
    }
}


// glm::mat4 normalMat = glm::transpose(glm::inverse(modelMatrix));
// glUniformMatrix4fv(shader.getUniform("u_normalMat"), 1, GL_FALSE, &normalMat[0][0]);
// glUniformMatrix4fv(shader.getUniform("u_modelMat"),     1, GL_FALSE, &modelMatrix[0][0]);
// glUniformMatrix4fv(shader.getUniform("u_viewMat"),      1, GL_FALSE, &viewMatrix[0][0]);
// glUniformMatrix4fv(shader.getUniform("u_projectionMat"),1, GL_FALSE, &projectionMatrix[0][0]);