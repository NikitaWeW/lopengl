#include "Model.hpp"
#include <stdexcept>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "opengl/VertexArray.hpp"
#include "opengl/IndexBuffer.hpp"
#include "logger.h"
#include "glad/gl.h"
#include "glm/gtc/matrix_transform.hpp"

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *material, aiTextureType const type, std::string const &typeName, bool flipTextures) { 
    std::vector<Texture> textures;
    aiString str;
    for(unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        material->GetTexture(type, i, &str);
        bool alreadyLoaded = false;
        for(auto &loadedTexture : m_loadedTextures)  {
            if(loadedTexture.getFilePath() == m_directory + '/' + str.C_Str()) {
                textures.push_back(loadedTexture);
                alreadyLoaded = true;
                break;
            }
        }
        if(!alreadyLoaded) {
            Texture texture{m_directory + '/' + str.C_Str(), flipTextures};
            texture.type = typeName;
            textures.push_back(texture);
            m_loadedTextures.push_back(texture);
        }
    }
    return textures;
}
void Model::processNode(aiNode *node, bool flipTextures) {
    for(unsigned i = 0; i < node->mNumMeshes; ++i) {
        m_meshes.push_back(processMesh(m_scene->mMeshes[node->mMeshes[i]], flipTextures));
    }
    for(unsigned i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], flipTextures);
    }
}
Mesh Model::processMesh(aiMesh *aimesh, bool flipTextures) {
    Mesh mesh;
    for(int i = 0; i < aimesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.position = { aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z };
        vertex.normal = { aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z };
        if(aimesh->mTextureCoords[0]) {
            vertex.textureCoords = { aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y };
        }
        mesh.vertices.push_back(vertex);
    }
    for(int i = 0; i < aimesh->mNumFaces; ++i) {
        aiFace face = aimesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; ++j)
            mesh.indices.push_back(face.mIndices[j]);
    }
    if(aimesh->mMaterialIndex >= 0) {
        aiMaterial *material = m_scene->mMaterials[aimesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse", flipTextures);
        mesh.textures.insert(mesh.textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular", flipTextures);
        mesh.textures.insert(mesh.textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "normal", flipTextures);
        mesh.textures.insert(mesh.textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "height", flipTextures);
        mesh.textures.insert(mesh.textures.end(), heightMaps.begin(), heightMaps.end());
        
        aiColor4D color;
        if(aiGetMaterialColor(material, AI_MATKEY_SHININESS, &color) == AI_SUCCESS) {
            mesh.material.shininess = color.r ? color.r : 32.0f;
        }
    }

    mesh.va.bind();
    mesh.vb = VertexBuffer{mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex)};
    mesh.ib = IndexBuffer {mesh.indices.data(),  mesh.indices.size()  * sizeof(unsigned)};
    mesh.va.addBuffer(mesh.vb, getVertexLayout());
    return mesh;
}

Model::Model(std::string const &filepath, bool flipTextures, bool flipWindingOrder) : m_filepath(filepath)
{
    if(!load(filepath, flipTextures, flipWindingOrder)) throw std::runtime_error("failed to import model!");
}

bool Model::load(const std::string &filepath, bool flipTextures, bool flipWindingOrder)
{
    Assimp::Importer importer;

    m_scene = importer.ReadFile( filepath,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_OptimizeMeshes         |
        aiProcess_OptimizeGraph          |
        aiProcess_SortByPType            |
        flipWindingOrder ? aiProcess_FlipWindingOrder : 0);

    if (!m_scene) {
        m_log = "error parcing \"" + filepath + "\": %s" + importer.GetErrorString();
        return false;
    } 
    m_directory = filepath.substr(0, filepath.find_last_of('/'));
    processNode(m_scene->mRootNode, flipTextures);
    return true;
}

void Model::resetMatrix()
{
    m_modelMat = glm::mat4{1.0f};
}

void Model::translate(glm::vec3 const &translation)
{
    m_modelMat = glm::translate(m_modelMat, translation);
}

void Model::rotate(glm::vec3 const &rotation) // eulers for now
{
    m_modelMat = glm::rotate(m_modelMat, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    m_modelMat = glm::rotate(m_modelMat, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    m_modelMat = glm::rotate(m_modelMat, glm::radians(rotation.z), glm::vec3(0, 0, 1));
}

void Model::scale(glm::vec3 const &scale)
{
    m_modelMat = glm::scale(m_modelMat, scale);
}

bool Model::operator==(Model const &other)
{
    return m_filepath == other.m_filepath;
}
