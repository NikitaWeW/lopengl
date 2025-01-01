#pragma once
#include "utils/Resource.hpp"
#include <vector>
#include <string>

class Cubemap : public Resource {
private:
    unsigned m_renderID = 0;
public:
    Cubemap(std::string directory, std::vector<std::string> faceTextureNames);
    ~Cubemap();

    void bind();
    void unbind();
};