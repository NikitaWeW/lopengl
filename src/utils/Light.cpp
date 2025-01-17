#include "Light.hpp"
#include "glad/gl.h"
#include "logger.h"
#include "glm/gtc/matrix_transform.hpp"

PointLight::PointLight()
{
    type = LightType::POINT;
    m_projMat = glm::mat4{1}; // TODO
}

glm::mat4 PointLight::getViewMatrix() const
{
    return glm::translate(glm::mat4{1}, position); // TODO
}

void PointLight::setUniforms(ShaderProgram const &shader, unsigned &pointLightCount, unsigned &dirLightCount, unsigned &spotLightCount) const
{
    shader.bind();
    std::string element = "u_pointLights[" + std::to_string(pointLightCount++) + ']';
    glUniform3fv(shader.getUniform(element + ".color"), 1,    &color.r);
    glUniform1f (shader.getUniform(element + ".constant"),     constant);
    glUniform1f (shader.getUniform(element + ".linear"),       linear);
    glUniform1f (shader.getUniform(element + ".quadratic"),    quadratic);
    glUniform3fv(shader.getUniform(element + ".position"), 1, &position.x);
    glUniformMatrix4fv(shader.getUniform(element + ".projectionMat"), 1, GL_FALSE, &m_projMat[0][0]);
    glUniformMatrix4fv(shader.getUniform(element + ".viewMat"), 1, GL_FALSE, &getViewMatrix()[0][0]);
}

DirectionalLight::DirectionalLight()
{
    type = LightType::DIRECTIONAL;
    m_projMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 10.0f);
}

glm::mat4 DirectionalLight::getViewMatrix() const
{
    return glm::lookAt(-direction, {0, 0, 0}, {0, 1, 0});
    // return glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
    //     glm::vec3( 0.0f, 0.0f, 0.0f),
    //     glm::vec3( 0.0f, 1.0f, 0.0f));
}

void DirectionalLight::setUniforms(ShaderProgram const &shader, unsigned &pointLightCount, unsigned &dirLightCount, unsigned &spotLightCount) const
{
    shader.bind();
    std::string element = "u_dirLights[" + std::to_string(dirLightCount++) + ']';
    glUniform3fv(shader.getUniform(element + ".color"), 1,    &color.r);
    glUniform1f (shader.getUniform(element + ".constant"),     constant);
    glUniform1f (shader.getUniform(element + ".linear"),       linear);
    glUniform1f (shader.getUniform(element + ".quadratic"),    quadratic);
    glUniform3fv(shader.getUniform(element + ".direction"), 1, &direction.x);
    glUniformMatrix4fv(shader.getUniform(element + ".projectionMat"), 1, GL_FALSE, &m_projMat[0][0]);
    glUniformMatrix4fv(shader.getUniform(element + ".viewMat"), 1, GL_FALSE, &getViewMatrix()[0][0]);
}

SpotLight::SpotLight()
{
    type = LightType::SPOT;
    // m_projMat = glm::perspective(); // TODO
}

glm::mat4 SpotLight::getViewMatrix() const
{
    return glm::mat4(); // TODO
}

void SpotLight::setUniforms(ShaderProgram const &shader, unsigned &pointLightCount, unsigned &dirLightCount, unsigned &spotLightCount) const
{
    shader.bind();
    std::string element = "u_spotLights[" + std::to_string(spotLightCount++) + ']';
    glUniform3fv(shader.getUniform(element + ".color"), 1,    &color.r);
    glUniform1f (shader.getUniform(element + ".constant"),     constant);
    glUniform1f (shader.getUniform(element + ".linear"),       linear);
    glUniform1f (shader.getUniform(element + ".quadratic"),    quadratic);
    glUniform3fv(shader.getUniform(element + ".position"), 1, &position.x);
    glUniform3fv(shader.getUniform(element + ".direction"),1, &direction.x);
    glUniform1f (shader.getUniform(element + ".innerCutoff"),  innerCutoff);
    glUniform1f (shader.getUniform(element + ".outerCutoff"),  outerCutoff);
    glUniformMatrix4fv(shader.getUniform(element + ".projectionMat"), 1, GL_FALSE, &m_projMat[0][0]);
    glUniformMatrix4fv(shader.getUniform(element + ".viewMat"), 1, GL_FALSE, &getViewMatrix()[0][0]);
}

glm::mat4 Light::getViewMatrix() const
{
    LOG_WARN("calling getViewMatrix for a base class");
    return glm::mat4();
}

void Light::setUniforms(ShaderProgram const &shader, unsigned &pointLightCount, unsigned &dirLightCount, unsigned &spotLightCount) const
{
    LOG_WARN("calling setUniforms for a base class");
}
