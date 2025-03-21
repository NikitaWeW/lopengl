#pragma once
#include "utils/Model.hpp"
#include "utils/AABB.hpp"
#include "opengl/ShaderStorageBuffer.hpp"
#include <map>

/*
* overcomplicated scene manager
*/
class Scene {
private:
    struct MeshInfo {
        std::string parentModel;
        Material *material;
        AABB aabb;

        unsigned indexOffset;
        unsigned vertexOffset;

        unsigned verticesCount;
        unsigned indicesCount;

        unsigned *indexData;
        glm::vec4 *positionData;
        glm::vec4 *normalData;
    };
    std::map<std::string, Model> m_models;
    std::vector<MeshInfo> m_meshInfos;
    SSBO m_indicesSSBO;
    SSBO m_positionsSSBO;
    SSBO m_normalsSSBO;
public:

    inline void addModel(std::string const &name, Model const &model) { m_models[name] = model; }
    inline void removeModel(std::string const &name) { m_models.erase(name); }
    inline std::map<std::string, Model> const &getModels() const { return m_models; }
    inline std::map<std::string, Model> &getModels() { return m_models; }
    inline std::vector<MeshInfo> const &getMeshInfo() const { return m_meshInfos; }
    inline SSBO const &getIndicesSSBO() const { return m_indicesSSBO; }
    inline SSBO const &getPositionsSSBO() const { return m_positionsSSBO; }
    inline SSBO const &getNormalsSSBO() const { return m_normalsSSBO; }
    /*
    * generate data required to draw the scene
    */
    void generateData();
    void setUniforms(std::string const &modelName, std::string const &uniformName, ShaderProgram const &shader, unsigned *numMeshesSet = nullptr, std::map<std::string, std::string> const &uniformStructLayout = {
        {"indexOffset", "indexOffset"},
        {"vertexOffset", "vertexOffset"},
        {"indicesCount", "indicesCount"},
        {"modelMat", "modelMat"},
        {"normalMat", "normalMat"},
        {"aabb.min", "aabb.min"},
        {"aabb.max", "aabb.max"},
        {"material.smoothness", "material.smoothness"},
        {"material.color", "material.color"},
        {"material.emission", "material.emission"},
        {"material.specularProbability", "material.specularProbability"},
        {"material.specularColor", "material.specularColor"}
    }) const;
};