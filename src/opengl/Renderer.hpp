/*
thats probably the worst class in this project. useless draw calls and m_lights vector that should not be there.
*/

#pragma once

#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include "utils/Model.hpp"
#include "utils/Light.hpp"
#include <optional>
#include "opengl/UniformBuffer.hpp"

class Renderer
{
private:
    std::vector<Light *> m_lights;
    
public:

public:
    Renderer();

    void clear(glm::vec3 clearColor = {0, 0, 0}) const;

    void setLightingUniforms(ShaderProgram const &shader) const;

    void draw(Mesh const &mesh) const;
    void draw(Model const &model) const;


    inline std::vector<Light *> &getLights() { return m_lights; }
    inline std::vector<Light *> const &getLights() const { return m_lights; }
};