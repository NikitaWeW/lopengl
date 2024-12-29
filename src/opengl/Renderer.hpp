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
/*
    self describing.
*/
    void clear(glm::vec3 clearColor = {0, 0, 0}) const;
/*
    set lighting and matrix uniforms before drawing.
*/
    void drawLighting(Model const &model, Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const;
    void drawLighting(Model const &model, Shader const &shader, Camera const &camera) const;
/*
    very basic draw call. no uniforms set.
*/
    void draw(Model const &model, Shader const &shader);

    inline std::vector<Light *> &getLights() { return m_lights; }
    inline std::vector<Light *> const &getLights() const { return m_lights; }
};