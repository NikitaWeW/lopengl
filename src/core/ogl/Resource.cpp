#include "Resource.hpp"
#include <utility>

Resource::Resource() = default;
Resource::Resource(Resource const &other)       { m_referenceCount = ++other.m_referenceCount; }
Resource::Resource(Resource &&other)            { m_referenceCount.exchange(other.m_referenceCount); }
void Resource::operator=(Resource const &other) { m_referenceCount = ++other.m_referenceCount; }
void Resource::operator=(Resource &&other)      { m_referenceCount.exchange(other.m_referenceCount); }

bool Resource::canDeallocate() const
{
    return m_referenceCount == 1;
}
