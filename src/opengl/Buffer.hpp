#pragma once
#include "utils/Resource.hpp"

class Buffer : public Resource {
protected:
    unsigned m_renderID = 0;
public:
    virtual void bufferData(void const *data, size_t size) const = 0;
    virtual void subdata(size_t offset, size_t size, void const *data) const = 0;
    virtual void allocate(size_t size) const = 0;
    virtual void deallocate() const = 0;
    virtual void*map(unsigned access) const = 0;
    virtual bool unmap() const = 0;
    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    inline unsigned getRenderID() const { return m_renderID; };
};