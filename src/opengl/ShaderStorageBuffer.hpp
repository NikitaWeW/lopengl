#pragma once
#include "utils/Resource.hpp"
#include <cstddef>

class SSBO : Resource {
private:
    unsigned m_renderID = 0;
public:
    SSBO();
    SSBO(size_t size);


    void bind(unsigned slot = 0) const;
    void unbind(unsigned slot = 0) const;
};