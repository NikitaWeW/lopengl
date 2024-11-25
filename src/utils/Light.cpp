#include "Light.hpp"
#include "glad/gl.h"
#include "logger.h"

PointLight::PointLight()
{
    type = LightType::POINT;
}

void PointLight::setUniforms(Shader const &shader, unsigned index) const
{
    shader.bind();
    std::string element = "u_pointLights[" + std::to_string(index) + ']';
    glUniform3fv(shader.getUniform(element + ".ambient"),  1, &ambient.r);
    glUniform3fv(shader.getUniform(element + ".diffuse"),  1, &diffuse.r);
    glUniform3fv(shader.getUniform(element + ".specular"), 1, &specular.r);
    glUniform1f (shader.getUniform(element + ".constant"),     constant);
    glUniform1f (shader.getUniform(element + ".linear"),       linear);
    glUniform1f (shader.getUniform(element + ".quadratic"),    quadratic);
    glUniform3fv(shader.getUniform(element + ".position"), 1, &position.x);
}

DirectionalLight::DirectionalLight()
{
    type = LightType::DIRECTIONAL;
}

void DirectionalLight::setUniforms(Shader const &shader, unsigned index) const
{
    shader.bind();
    std::string element = "u_dirLights[" + index + ']';
    glUniform3fv(shader.getUniform(element + ".ambient"),  1, &ambient.r);
    glUniform3fv(shader.getUniform(element + ".diffuse"),  1, &diffuse.r);
    glUniform3fv(shader.getUniform(element + ".specular"), 1, &specular.r);
    glUniform1f (shader.getUniform(element + ".constant"),     constant);
    glUniform1f (shader.getUniform(element + ".linear"),       linear);
    glUniform1f (shader.getUniform(element + ".quadratic"),    quadratic);
    glUniform3fv(shader.getUniform(element + ".direction"), 1, &direction.x);
}

SpotLight::SpotLight()
{
    type = LightType::SPOT;
}

void SpotLight::setUniforms(Shader const &shader, unsigned index) const
{
    shader.bind();
    std::string element = "u_spotLights[" + index + ']';
    glUniform3fv(shader.getUniform(element + ".ambient"),  1, &ambient.r);
    glUniform3fv(shader.getUniform(element + ".diffuse"),  1, &diffuse.r);
    glUniform3fv(shader.getUniform(element + ".specular"), 1, &specular.r);
    glUniform1f (shader.getUniform(element + ".constant"),     constant);
    glUniform1f (shader.getUniform(element + ".linear"),       linear);
    glUniform1f (shader.getUniform(element + ".quadratic"),    quadratic);
    glUniform3fv(shader.getUniform(element + ".position"), 1, &position.x);
    glUniform3fv(shader.getUniform(element + ".direction"),1, &direction.x);
    glUniform1f (shader.getUniform(element + ".innerCutoff"),  innerCutoff);
    glUniform1f (shader.getUniform(element + ".outerCutoff"),  outerCutoff);
}

void Light::setUniforms(Shader const &shader, unsigned index) const
{
    LOG_WARN("calling setUniforms for a base class (not good)");
}
