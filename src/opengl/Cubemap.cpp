#include "Cubemap.hpp"
#include "stb_image.h"
#include "glad/gl.h"
#include "logger.h"
#include "stdexcept"
#include <filesystem>

Cubemap::Cubemap(std::string directory, std::vector<std::string> faceTextureNames)
{
    glGenTextures(1, &m_renderID);
    bind();
    int width, height, BPP;
    for(unsigned i = 0; i < faceTextureNames.size(); ++i) {
        std::string filepath = directory + '/' + faceTextureNames[i];
        unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &BPP, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            LOG_ERROR("failed to load texture at \"%s\". Reason: %s", filepath.c_str(), stbi_failure_reason());
            stbi_image_free(data);
            throw std::runtime_error("failed to load texture!");
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Cubemap::~Cubemap()
{
    if(canDeallocate()) {
        glDeleteTextures(1, &m_renderID);
    }
}

void Cubemap::bind(unsigned slot)   { glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderID); }
void Cubemap::unbind(unsigned slot) { glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }
