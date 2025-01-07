#include "Light.hpp"
#include "glad/gl.h"
#include "logger.h"

PointLight::PointLight()
{
    type = LightType::POINT;
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
}

DirectionalLight::DirectionalLight()
{
    type = LightType::DIRECTIONAL;
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
}

SpotLight::SpotLight()
{
    type = LightType::SPOT;
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
}

void Light::setUniforms(ShaderProgram const &shader, unsigned &pointLightCount, unsigned &dirLightCount, unsigned &spotLightCount) const
{
    LOG_WARN("calling setUniforms for a base class (not good)");
}
