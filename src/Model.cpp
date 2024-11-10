#include "glad/gl.h"
#include "Model.hpp"
#include <stdexcept>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "opengl/VertexArray.hpp"
#include "opengl/IndexBuffer.hpp"
#include "logger.h"
<<<<<<< Updated upstream:src/Model.cpp
#include "glad/gl.h"
// 0 AI!!
=======
#include "glm/gtc/matrix_transform.hpp"
>>>>>>> Stashed changes:src/utils/Model.cpp

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    aiString str;
    for(unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        material->GetTexture(type, i, &str);
        bool alreadyLoaded = false;
        for(auto &loadedTexture : m_loadedTextures)  {
            if(loadedTexture.getFilePath() == directory + '/' + str.C_Str()) {
                textures.push_back(loadedTexture);
                alreadyLoaded = true;
                break;
            }
        }
        if(!alreadyLoaded) {
            Texture texture{directory + '/' + str.C_Str(), false};
            texture.type = typeName;
            textures.push_back(texture);
            m_loadedTextures.push_back(texture);
        }
    }
    return textures;
}
void Model::processNode(aiNode *node) {
    for(unsigned i = 0; i < node->mNumMeshes; ++i) {
        m_meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]]));
    }
    for(unsigned i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i]);
    }
}
Mesh Model::processMesh(aiMesh *aimesh) {
    Mesh mesh;
    for(int i = 0; i < aimesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.position = { aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z };
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
        aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        mesh.textures.insert(mesh.textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        mesh.textures.insert(mesh.textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        mesh.textures.insert(mesh.textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        mesh.textures.insert(mesh.textures.end(), heightMaps.begin(), heightMaps.end());
        
        aiColor4D color;

        if(aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &color) == AI_SUCCESS) 
            mesh.material.ambient = {color.r, color.g, color.b};
        if(aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) 
            mesh.material.diffuse = {color.r, color.g, color.b};
        if(aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &color) == AI_SUCCESS) 
            mesh.material.specular = {color.r, color.g, color.b};
        if(aiGetMaterialColor(material, AI_MATKEY_SHININESS, &color) == AI_SUCCESS) 
            mesh.material.shininess = color.r;
    }
    
    mesh.va.bind();
    mesh.vb = VertexBuffer {mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex)};
    mesh.ib = IndexBuffer  {mesh.indices.data(),  mesh.indices.size()  * sizeof(unsigned)};
    mesh.va.addBuffer(mesh.vb, meshLayout);
    return mesh;
}

void Model::draw(Shader const &shader) {
    shader.bind();
    for(Mesh &mesh : m_meshes) {
        mesh.va.bind();
        mesh.ib.bind();
        unsigned int diffuseNr  = 0; // some dark magic here
        unsigned int specularNr = 0; // set shader unforms typeN, etc. texture_diffuse0
        unsigned int normalNr   = 0;
        unsigned int heightNr   = 0;
        for(unsigned i = 0; i < mesh.textures.size(); ++i) {
            std::string &type = mesh.textures[i].type;
            unsigned n; // typeN <- texure index
              if(type == "texture_diffuse") {
                n = diffuseNr++;
            } else if(type == "texture_specular") {
                n = specularNr++;
            } else if(type == "texture_normal") {
                n = normalNr++;
            } else if(type == "texture_height") {
                n = heightNr++;
            }
            if(shader.getUniform(type + std::to_string(n)) != -1) glUniform1i(shader.getUniform(type + std::to_string(n)), i);
            mesh.textures[i].bind(i);
        }
<<<<<<< Updated upstream:src/Model.cpp
=======
        glm::mat4 normalMat = glm::transpose(glm::inverse(m_modelMat));
        if(shader.getUniform("u_material.ambient") != -1)  glUniform3fv(shader.getUniform("u_material.ambient"),  1, &mesh.material.ambient.r);
        if(shader.getUniform("u_material.diffuse") != -1)  glUniform3fv(shader.getUniform("u_material.diffuse"),  1, &mesh.material.diffuse.r);
        if(shader.getUniform("u_material.specular") != -1) glUniform3fv(shader.getUniform("u_material.specular"), 1, &mesh.material.specular.r);
        if(shader.getUniform("u_material.shininess") != -1)glUniform1f (shader.getUniform("u_material.shininess"),    mesh.material.shininess);

        if(shader.getUniform("u_model") != -1)     glUniformMatrix4fv(shader.getUniform("u_model"),     1, GL_FALSE, &m_modelMat[0][0]);
        if(shader.getUniform("u_view") != -1)      glUniformMatrix4fv(shader.getUniform("u_view"),      1, GL_FALSE, &viewMat[0][0]);
        if(shader.getUniform("u_projection") != -1)glUniformMatrix4fv(shader.getUniform("u_projection"),1, GL_FALSE, &projectionMat[0][0]);
        if(shader.getUniform("u_normalMat") != -1) glUniformMatrix4fv(shader.getUniform("u_normalMat"), 1, GL_FALSE, &normalMat[0][0]);
>>>>>>> Stashed changes:src/utils/Model.cpp
        glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        mesh.va.unbind();
        mesh.ib.unbind();
    }
    glActiveTexture(GL_TEXTURE0);
}

Model::Model(std::string const &filepath) : filepath(filepath)
{
    Assimp::Importer importer;

    scene = importer.ReadFile( filepath,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!scene) {
        LOG_ERROR("error parcing \"%s\": %s", filepath.c_str(), importer.GetErrorString());
        throw std::runtime_error("failed to import model!");
    } 
    directory = filepath.substr(0, filepath.find_last_of('/'));
    meshLayout = {};
    meshLayout.push(3, GL_FLOAT);
    meshLayout.push(2, GL_FLOAT);
    meshLayout.push(1, GL_FLOAT);
    processNode(scene->mRootNode);
}
Model::~Model()
{
}
