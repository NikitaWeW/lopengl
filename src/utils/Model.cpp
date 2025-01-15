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
        for(auto &loadedTexture : m_loadedTextures) {
            if(loadedTexture.getFilePath() == m_directory + '/' + str.C_Str()) {
                textures.push_back(loadedTexture);
                alreadyLoaded = true;
                break;
            }
        }
        if(!alreadyLoaded) {
            std::string filepath{m_directory + '/' + str.C_Str()};
            std::replace_if(filepath.begin(), filepath.end(), [](char c){ return c == '\\'; }, '/');
            Texture texture{filepath, flipTextures, type == aiTextureType_DIFFUSE};
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
    std::vector<unsigned>  indices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textureCoords;

    for(unsigned i = 0; i < aimesh->mNumVertices; ++i) {
        positions.push_back({ aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z });
        normals.push_back({ aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z });
        if(aimesh->mTextureCoords[0]) {
            textureCoords.push_back({ aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y });
        }
    }
    mesh.vb = VertexBuffer{
        positions.size()     * sizeof(positions[0]) + 
        normals.size()       * sizeof(normals[0]) + 
        textureCoords.size() * sizeof(textureCoords[0])
    };

    mesh.ib.bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0,                                     positions.size() * sizeof(positions[0]), positions.data());
    glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), normals.size() * sizeof(normals[0]), normals.data());
    if(aimesh->mTextureCoords[0]) 
        glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]) + normals.size() * sizeof(normals[0]), textureCoords.size() * sizeof(textureCoords[0]), textureCoords.data());

    for(unsigned i = 0; i < aimesh->mNumFaces; ++i) {
        aiFace face = aimesh->mFaces[i];
        for(unsigned j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
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

    VertexBufferLayout layout;
    /*  positions */ layout.push(3, GL_FLOAT, 0);
    /*   normals  */ layout.push(3, GL_FLOAT, positions.size() * sizeof(positions[0]));
    /* tex coords */ layout.push(2, GL_FLOAT, positions.size() * sizeof(positions[0]) + normals.size()   * sizeof(normals[0]));
    
    mesh.va.bind();
    mesh.ib = IndexBuffer {indices.data(),  indices.size()  * sizeof(unsigned)};
    mesh.va.addBuffer(mesh.vb, layout);
    return mesh;
}

Model::Model(std::string const &filepath, bool flipTextures, bool flipWindingOrder) : m_filepath(filepath)
{
    if(!load(filepath, flipTextures, flipWindingOrder)) throw std::runtime_error("failed to import model!");
}

bool Model::load(std::string const &filepath, bool flipTextures, bool flipWindingOrder) {

    Assimp::Importer importer;

    m_scene = importer.ReadFile( filepath,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType            |
        aiProcess_OptimizeGraph          |
        aiProcess_OptimizeMeshes         |
        (flipWindingOrder ? aiProcess_FlipWindingOrder : 0)
    );

    if (!m_scene) {
        LOG_ERROR("error parcing \"%s\": %s", filepath.c_str(), importer.GetErrorString());
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
