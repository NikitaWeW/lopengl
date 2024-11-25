#pragma once

#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include "utils/Model.hpp"
#include "utils/Light.hpp"
#include <optional>

class Renderer
{
private:
    std::vector<Light *> m_lights;
public:

public:
    Renderer();
    void clear(float r = 0, float g = 0, float b = 0) const;
    void draw(Model const &model, Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const;
    void draw(Model const &model, Shader const &shader, Camera const &camera) const;
    void draw(Model const &model) const;
    inline std::vector<Light *> &getLights() { return m_lights; }
    inline std::vector<Light *> const &getLights() const { return m_lights; }
};