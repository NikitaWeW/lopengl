#pragma once

/*
deallocate only if canDeallocate returns true
*/
class Resource {
private:
    mutable bool m_managing = false;
public:
    Resource();
    Resource(Resource const &other);
    Resource(Resource &&other);

    ~Resource() = default;

    void operator=(Resource const &other);

    inline bool canDeallocate() const { return m_managing; }
};