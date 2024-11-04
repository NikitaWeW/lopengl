#pragma once
#include <string>
#include <vector>
#include "opengl/Vertex.hpp"
#include "opengl/Texture.hpp"
#include "assimp/scene.h"
#include "opengl/Shader.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/VertexArray.hpp"

struct Mesh {
    Mesh() = default;
    VertexBuffer vb;
    IndexBuffer ib;
    VertexArray va;

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;
};
class Model { 
private:
    aiScene const* scene;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_loadedTextures;
    std::string directory;
    VertexBufferLayout meshLayout;
private:
    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName);
    void processNode(aiNode *node);
    Mesh processMesh(aiMesh *aimesh);
public:
    void draw(Shader const &shader);
    Model(const std::string &filepath);
    ~Model();
};

