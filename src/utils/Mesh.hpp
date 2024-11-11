#pragma once
#include "Material.hpp"
#include "Vertex.hpp"
#include "opengl/Texture.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/VertexArray.hpp"

struct Mesh {
    Material material;

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;

    VertexBuffer vb;
    IndexBuffer ib;
    VertexArray va;
};