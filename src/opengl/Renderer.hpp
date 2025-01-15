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

    // clears all buffers with clearColor color
    void clear(glm::vec3 clearColor = {0, 0, 0}) const;


    void setLightingUniforms(ShaderProgram const &shader) const;
    void setMaterialUniforms(ShaderProgram const &shader, Mesh const &mesh) const;
    void setMaterialUniforms(ShaderProgram const &shader, int const diffuse, int const specular = -1) const;
    void setMatrixUniforms(ShaderProgram const &shader, glm::mat4 const &modelMatrix, Camera const &camera) const;
    void setMatrixUniforms(ShaderProgram const &shader, glm::mat4 const &modelMatrix, glm::mat4 const &viewMatrix, glm::mat4 const &projectionMatrix) const;

    // draw a mesh. no uniforms set.
    void draw(Mesh const &mesh, ShaderProgram const &shader) const;
    // draw a model. all uniforms set.
    void draw(Model const &model, ShaderProgram const &shader, Camera const &camera) const;
    // draw a model. no uniforms set.
    void drawb(Model const &model, ShaderProgram const &shader, Camera const &camera) const;

    inline std::vector<Light *> &getLights() { return m_lights; }
    inline std::vector<Light *> const &getLights() const { return m_lights; }
};