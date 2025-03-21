#include "Scene.hpp"

void Scene::generateData()
{
    unsigned numIndices = 0;
    unsigned numVertices = 0;
    for(auto modelIterator = m_models.begin(); modelIterator != m_models.end(); ++modelIterator) {
        for(auto meshIterator = modelIterator->second.getMeshes().begin(); meshIterator != modelIterator->second.getMeshes().end(); ++meshIterator) {
            MeshInfo meshInfo;
            for(unsigned index : meshIterator->indices) {
                meshInfo.aabb.growToInclude(meshIterator->positions[index]);
            }
            meshInfo.indexOffset = numIndices;
            meshInfo.vertexOffset = numVertices;

            meshInfo.indicesCount = meshIterator->indices.size();
            meshInfo.verticesCount = meshIterator->positions.size();

            meshInfo.indexData = meshIterator->indices.data();
            meshInfo.positionData = meshIterator->positions.data();
            meshInfo.normalData = meshIterator->normals.data();

            meshInfo.parentModel = modelIterator->first;
            meshInfo.material = &meshIterator->material;

            m_meshInfos.push_back(meshInfo);

            numIndices += meshInfo.indicesCount;
            numVertices += meshInfo.verticesCount;
        }
    }

    m_indicesSSBO = SSBO{numIndices * sizeof(unsigned)};
    m_positionsSSBO = SSBO{numVertices * sizeof(glm::vec4)};
    m_normalsSSBO = SSBO{numVertices * sizeof(glm::vec4)};
    for(auto infoIterator = m_meshInfos.begin(); infoIterator != m_meshInfos.end(); ++infoIterator) {
        m_indicesSSBO.bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
            infoIterator->indexOffset * sizeof(unsigned), 
            infoIterator->indicesCount * sizeof(unsigned), 
            infoIterator->indexData
        );
        m_positionsSSBO.bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
            infoIterator->vertexOffset * sizeof(glm::vec4), 
            infoIterator->verticesCount * sizeof(glm::vec4), 
            infoIterator->positionData
        );
        m_normalsSSBO.bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
            infoIterator->vertexOffset * sizeof(glm::vec4), 
            infoIterator->verticesCount * sizeof(glm::vec4), 
            infoIterator->normalData
        );
    }
}

void Scene::setUniforms(std::string const &modelName, std::string const &uniformName, ShaderProgram const &shader, unsigned *numMeshesSet, std::map<std::string, std::string> const &uniformStructLayout) const
{ // TODO: uniform buffer
    for(MeshInfo const &info : m_meshInfos) {
        if(info.parentModel != modelName) continue;
        glUniform1ui(shader.getUniform(uniformName + "." + uniformStructLayout.at("indicesCount")), info.indicesCount);
        glUniform1ui(shader.getUniform(uniformName + "." + uniformStructLayout.at("indexOffset")), info.indexOffset);
        glUniform1ui(shader.getUniform(uniformName + "." + uniformStructLayout.at("vertexOffset")), info.vertexOffset);
        glUniform3fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("aabb.min")), 1, &info.aabb.min.x);
        glUniform3fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("aabb.max")), 1, &info.aabb.max.x);
        glUniform3fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("material.color")), 1, &info.material->color.r);
        glUniform3fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("material.specularColor")), 1, &info.material->specularColor.r);
        glUniform3fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("material.emission")), 1, &info.material->emission.r);
        glUniform1f (shader.getUniform(uniformName + "." + uniformStructLayout.at("material.smoothness")), info.material->smoothness);
        glUniform1f (shader.getUniform(uniformName + "." + uniformStructLayout.at("material.specularProbability")), info.material->specularProbability);
        glUniformMatrix4fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("modelMat")), 1, GL_FALSE, &m_models.at(modelName).getModelMat()[0][0]);
        glUniformMatrix4fv(shader.getUniform(uniformName + "." + uniformStructLayout.at("normalMat")), 1, GL_FALSE, &glm::transpose(glm::inverse(m_models.at(modelName).getModelMat()))[0][0]);
        if(numMeshesSet) ++(*numMeshesSet);
    }
}
