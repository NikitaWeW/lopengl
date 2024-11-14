#include "Model.hpp"
#include <stdexcept>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "opengl/VertexArray.hpp"
#include "opengl/IndexBuffer.hpp"
#include "logger.h"
#include "glad/gl.h"
#include "glm/gtc/matrix_transform.hpp"

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *material, aiTextureType const type, std::string const &typeName) { 
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
            Texture texture{m_directory + '/' + str.C_Str(), false};
            texture.type = typeName;
            textures.push_back(texture);
            m_loadedTextures.push_back(texture);
        }
    }
    return textures;
}
void Model::processNode(aiNode *node) {
    for(unsigned i = 0; i < node->mNumMeshes; ++i) {
        m_meshes.push_back(processMesh(m_scene->mMeshes[node->mMeshes[i]]));
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
        vertex.normals = { aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z };
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

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
        mesh.textures.insert(mesh.textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
        mesh.textures.insert(mesh.textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "normal");
        mesh.textures.insert(mesh.textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "height");
        mesh.textures.insert(mesh.textures.end(), heightMaps.begin(), heightMaps.end());
        
        aiColor4D color;
        if(aiGetMaterialColor(material, AI_MATKEY_SHININESS, &color) == AI_SUCCESS) {
            mesh.material.shininess = color.r ? color.r : 32.0f;
        }
    }
    
    mesh.va.bind();
    mesh.vb = VertexBuffer{mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex)};
    mesh.ib = IndexBuffer {mesh.indices.data(),  mesh.indices.size()  * sizeof(unsigned)};
    mesh.va.addBuffer(mesh.vb, m_meshLayout);
    return mesh;
}

void Model::draw(Shader const &shader, glm::mat4 const &viewMat, glm::mat4 const &projectionMat) const {
    shader.bind();
    for(Mesh const &mesh : m_meshes) {
        mesh.va.bind();
        mesh.ib.bind();
        std::vector<unsigned> texturesToUnbind; // wanna unbind textuers after render
        if(mesh.textures.size() == 0) {
            glUniform1i(shader.getUniform("u_material.diffuse"), 0);
        } else {
            unsigned int textureCount = 1; // leave 0 for other purposes
            for(Texture const &texture : mesh.textures) {
                unsigned location = shader.getUniform("u_material." + texture.type);
                if(location != -1) {
                    glUniform1i(location, textureCount);
                    texture.bind(textureCount);
                    texturesToUnbind.push_back(textureCount);
                    ++textureCount;
                }
            }
        }
        glUniform1f(shader.getUniform("u_material.shininess"), mesh.material.shininess);
        glm::mat4 normalMat = glm::transpose(glm::inverse(m_modelMat));
        glUniformMatrix4fv(shader.getUniform("u_model"),     1, GL_FALSE, &m_modelMat[0][0]);
        glUniformMatrix4fv(shader.getUniform("u_view"),      1, GL_FALSE, &viewMat[0][0]);
        glUniformMatrix4fv(shader.getUniform("u_projection"),1, GL_FALSE, &projectionMat[0][0]);
        glUniformMatrix4fv(shader.getUniform("u_normalMat"), 1, GL_FALSE, &normalMat[0][0]);
        glDrawElements(GL_TRIANGLES, mesh.ib.getSize(), GL_UNSIGNED_INT, nullptr);
        mesh.va.unbind();
        mesh.ib.unbind();
        for(unsigned slot : texturesToUnbind) {
            Texture::unbindStatic(slot);
        }
    }
    glActiveTexture(GL_TEXTURE0);
}

void Model::draw(Shader const &shader, Camera const &camera, int const windowWidth, int const windowHeight) const
{
    draw(shader, camera.getViewMatrix(), camera.getProjectionMatrix(windowWidth, windowHeight));
}

Model::Model(std::string const &filepath) : m_filepath(filepath)
{
    Assimp::Importer importer;

    m_scene = importer.ReadFile( filepath,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!m_scene) {
        LOG_ERROR("error parcing \"%s\": %s", filepath.c_str(), importer.GetErrorString());
        throw std::runtime_error("failed to import model!");
    } 
    m_directory = filepath.substr(0, filepath.find_last_of('/'));
    m_meshLayout = getVertexLayout();
    processNode(m_scene->mRootNode);
}
Model::~Model()
{
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
