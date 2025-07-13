#pragma once
#include "Resource.hpp"

namespace ogl 
{
    class Object : public Resource 
    {
    protected:
        unsigned m_renderID = 0;
    public:
        inline bool canDeallocate() const { return Resource::canDeallocate() && m_renderID; }
        inline unsigned getRenderID() const noexcept { return m_renderID; }
        virtual void bind(unsigned slot = 0) const noexcept = 0;
    };
}; // namespace ogl