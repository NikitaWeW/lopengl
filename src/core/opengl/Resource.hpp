#pragma once
#include <memory>
#include <atomic>

/*
deallocate only if canDeallocate returns true
*/
class Resource 
{
private:
    mutable std::atomic_uint m_referenceCount = 1;
public:
    Resource();
    Resource(Resource const &other);
    Resource(Resource &&other);
    void operator=(Resource const &other);
    void operator=(Resource &&other);

    // object owns the resource
    bool canDeallocate() const;
    virtual ~Resource() = default;
};