#pragma once
#include "Material.hpp"
#include "Vertex.hpp"
#include "opengl/Texture.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/ShaderStorageBuffer.hpp"

struct Mesh {
    std::vector<Texture> textures;
    std::vector<unsigned>  indices;
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec4> tangents;
    std::vector<glm::vec2> textureCoords;
    Material material;
};