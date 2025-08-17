#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class mm
{
private:
    glm::mat4 m_modelMat{1.0f};
public:
    mm() = default;
    inline mm &reset()
    {
        m_modelMat = glm::mat4{1.0f};
        return *this;
    }
    inline mm &translate(glm::vec3 v) 
    { 
        m_modelMat = glm::translate(m_modelMat, v); 
        return *this;
    }
    inline mm &rotate(glm::vec3 v) 
    {
        m_modelMat = glm::rotate(m_modelMat, v.x, glm::vec3{1, 0, 0});
        m_modelMat = glm::rotate(m_modelMat, v.y, glm::vec3{0, 1, 0});
        m_modelMat = glm::rotate(m_modelMat, v.z, glm::vec3{0, 0, 1});
        return *this;
    }
    inline mm &scale(glm::vec3 v) 
    {
        m_modelMat = glm::scale(m_modelMat, v); 
        return *this;
    }
    inline mm &translate(float x, float y, float z) 
    { 
        return translate(glm::vec3{x, y, z});
    }
    inline mm &rotate(float x, float y, float z) 
    {
        return rotate(glm::vec3{x, y, z});
    }
    inline mm &scale(float x, float y, float z) 
    {
        return scale(glm::vec3{x, y, z});
    }
    inline mm &rotate(float a, glm::vec3 v) 
    {
        return *this;
    }
    inline glm::mat4 get() const 
    {
        return m_modelMat;
    }
};
