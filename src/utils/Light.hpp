#pragma once
#include "glm/glm.hpp"
#include "opengl/Shader.hpp"

enum LightType {
    POINT, DIRECTIONAL, SPOT, NONE = 0
};

struct Light {
    LightType type = NONE;
    glm::vec3 ambient = glm::vec3{0};
    glm::vec3 diffuse = glm::vec3{1};
    glm::vec3 specular = glm::vec3{0};

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    // Light() = default;
    // Light(Light &other) = default;
    // Light(Light &&other) = default;
    // Light &operator=(Light &other) = default;
    // Light &operator=(Light &&other) = default;
    virtual ~Light() = default;
    virtual void setUniforms(Shader const &shader, unsigned index) const;
};

struct PointLight : public Light {
    PointLight();
    glm::vec3 position = glm::vec3{0};
    virtual void setUniforms(Shader const &shader, unsigned index) const;
};

struct DirectionalLight : public Light {
    DirectionalLight();
    glm::vec3 direction = glm::vec3{1, 0, 0};
    virtual void setUniforms(Shader const &shader, unsigned index) const;
};

struct SpotLight : public Light {
    SpotLight();
    glm::vec3 position = glm::vec3{0};
    glm::vec3 direction = glm::vec3{0, 0, -1};
    float innerCutoff = glm::cos(glm::radians(20.0f));
    float outerCutoff = glm::cos(glm::radians(40.0f));
    virtual void setUniforms(Shader const &shader, unsigned index) const;
};