#include "Vertex.hpp"
#include "Material.hpp"
#include "opengl/Texture.hpp"
#include <vector>

struct Mesh {
    VertexBuffer vb;
    IndexBuffer ib;
    VertexArray va;
    Material material;

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;
};