#pragma once
#include <string>
#include <vector>
#include "assimp/scene.h"
#include "opengl/Shader.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

class Model { 
private:
    aiScene const *m_scene;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_loadedTextures;
    std::string m_directory;
    std::string m_filepath;
    glm::mat4 m_modelMat;
private:
    std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType const type, std::string const &typeName, bool flipTextures);
    void processNode(aiNode *node, bool flipTextures);
    Mesh processMesh(aiMesh *aimesh, bool flipTextures);

public:
    glm::vec3 m_position = glm::vec3{0}; // does not do anythyng.
    glm::vec3 m_rotation = glm::vec3{0};
    glm::vec3 m_scale = glm::vec3{1};
public:
    Model() = default;
    Model(const std::string &filepath, bool flipTextures = false, bool flipWindingOrder = false);
    bool load(std::string const &filepath, bool flipTextures = false, bool flipWindingOrder = false);
    ~Model() = default;
    void resetMatrix();
    void translate(glm::vec3 const &translation);
    void rotate(glm::vec3 const &rotation);
    void scale(glm::vec3 const &scale);
    
    inline void translate() { translate(m_position); }
    inline void rotate() { rotate(m_rotation); }
    inline void scale() { scale(m_scale); }

    bool operator==(Model const &other);
    inline aiScene const *getScene() const { return m_scene; }
    inline std::string const &getFilepath() const { return m_filepath; }
    inline std::vector<Mesh> const &getMeshes() const { return m_meshes; }
    inline std::vector<Mesh> &getMeshes() { return m_meshes; }
    inline glm::mat4 const &getModelMat() const { return m_modelMat; }
    inline glm::mat4 &getModelMat() { return m_modelMat; }
};

