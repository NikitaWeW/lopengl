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
#include "Camera.hpp"

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
    aiScene const *m_scene;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_loadedTextures;
    std::string m_directory;
    VertexBufferLayout m_meshLayout;
    std::string m_filepath;
    glm::mat4 m_modelMat;
private:
    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName);
    void processNode(aiNode *node);
    Mesh processMesh(aiMesh *aimesh);
public:
    void draw(Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const;
    void draw(Shader const &shader, Camera const &camera, int const windowWidth, int const windowHeight) const;
    Model(const std::string &filepath);
    ~Model();
    void resetMatrix();
    void translate(glm::vec3 const &translation);
    void rotate(glm::vec3 const &rotation);
    void scale(glm::vec3 const &scale);
    bool operator==(Model const &other);
    inline aiScene const *getScene() const { return m_scene; }
    inline std::string const &getFilepath() const { return m_filepath; }
    inline std::vector<Mesh> const &getMeshes() const { return m_meshes; }
    inline std::vector<Mesh> &getMeshes() { return m_meshes; }
    inline glm::mat4 const &getModelMat() const { return m_modelMat; }
    inline glm::mat4 &getModelMat() { return m_modelMat; }
};

