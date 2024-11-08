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
    VertexBuffer vb;
    IndexBuffer ib;
    VertexArray va;

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;
};
class Model { 
private:
    aiScene const *scene;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_loadedTextures;
    std::string directory;
    VertexBufferLayout meshLayout;
    std::string filepath;
private:
    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName);
    void processNode(aiNode *node);
    Mesh processMesh(aiMesh *aimesh);
public:
    void draw(Shader const &shader);
    Model(const std::string &filepath);
    ~Model();
    bool operator==(Model const &other);
    inline aiScene const *getScene() const {
        return scene;
    }
    inline std::string const &getFilepath() const {
        return filepath;
    }
    inline std::vector<Mesh> const &getMeshes() const {
        return m_meshes;
    }
    inline std::vector<Mesh> &getMeshes() {
        return m_meshes;
    }
};

