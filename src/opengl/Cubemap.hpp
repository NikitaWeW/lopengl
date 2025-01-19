#pragma once
#include "utils/Resource.hpp"
#include <vector>
#include <string>
#include "glad/gl.h"

class Cubemap : public Resource {
private:
    unsigned m_renderID = 0;
public:
    Cubemap(std::string directory, std::vector<std::string> faceTextureNames);
    Cubemap(GLenum wrap = GL_CLAMP_TO_EDGE, GLenum filter = GL_NEAREST);
    ~Cubemap();

    void bind(unsigned slot = 0);
    void unbind(unsigned slot = 0);

    inline unsigned getRenderID() { return m_renderID; }
};