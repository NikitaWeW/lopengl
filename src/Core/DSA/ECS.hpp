// TODO: merge this into the engine

#pragma once
#include "nicecs/ecs.hpp"
#include <cstdint>
#include <shared_mutex>

class Entity;

template<typename... T>
using exclude = ecs::exclude<T...>;

/// @brief A thin layer above the ecs::registry with support for thread synchronization.
class Registry
{
private:
    mutable std::shared_mutex mMutex;
    ecs::registry mReg;
public:
    Registry() = default;
    inline explicit Registry(ecs::registry &&reg) : mReg(std::move(reg)) {};

    /// @brief Get an underlying ecs::registry
    inline ecs::registry &getReg() { return mReg; }
    /// @brief Get an underlying ecs::registry
    inline ecs::registry const &getReg() const { return mReg; }

    inline ecs::registry const *operator->() const { return &getReg(); }
    inline ecs::registry *operator->() { return &getReg(); }

    /// @brief Get the shared mutex.
    inline std::shared_mutex &getMutex() const { return mMutex; }

    /// @copydoc ecs::registry::create
    template <typename... Components_t> 
    Entity create();

    /// @copydoc ecs::registry::create
    template <typename... Components_t> 
    Entity create(Components_t&&... components);

    /// @copydoc ecs::registry::destroy
    void destroy(Entity const &entity);

    /// @copydoc ecs::registry::size
    std::size_t size() const;

    /// @copydoc ecs::registry::view
    template<typename... Include, typename... Exclude>
    std::vector<Entity> view(exclude<Exclude...> toExclude = exclude{}) const;

    /// @copydoc ecs::registry::viewAny
    template<typename... May, typename... Exclude>
    std::vector<Entity> viewAny(exclude<Exclude...> toExclude = exclude{}) const;

    /// @copydoc ecs::registry::merged
    Registry merged(Registry const &other) const;
    /// @copydoc ecs::registry::merged
    void merge(Registry const &other);
};

/// @brief A helper class to group the entity and the registry it belongs to.
/// Invalidates if the registry is moved or if the entity invalidates obviously.
class Entity
{
private:
    Registry *mReg = nullptr;
    ecs::entity mEntity = 0;
public:
    inline constexpr Entity() = default;
    inline Entity(Registry *reg, ecs::entity e) : mReg(reg), mEntity(e) {}
    inline ecs::entity entity() const { return mEntity; }
    inline Registry const &reg() const 
    { 
        assert(mReg && "Invalid registry!");
        return *mReg; 
    }
    inline Registry &reg() 
    { 
        assert(mReg && "Invalid registry!");
        return *mReg; 
    }

    template <typename component_t, class... Args>
    inline void emplace(Args&&... args) { return reg()->emplace<component_t, Args...>(entity(), std::forward<Args>(args)...); }

    /// @copydoc ecs::registry::has
    template <typename component_t> 
    inline bool has() const { return reg()->has<component_t>(entity()); }

    /// @copydoc ecs::registry::get
    template <typename component_t> 
    inline component_t const &get() const { return reg()->get<component_t>(entity()); }
    /// @copydoc ecs::registry::get
    template <typename component_t> 
    inline component_t &get() { return reg()->get<component_t>(entity()); }

    /// @copydoc ecs::registry::remove
    template <typename component_t> 
    inline void remove() { return reg()->remove<component_t>(entity()); }

    /// @copydoc ecs::registry::size
    inline std::size_t size() const { return reg()->size(entity()); }

    /// @copydoc ecs::registry::valid
    /// Also checks if the registry is valid (not nullptr)
    inline bool valid() const { return mReg && reg()->valid(entity()); }

    inline bool operator==(Entity const &o) const { return mEntity == o.mEntity && mReg == o.mReg; }
};

template <typename... Components_t> 
inline Entity Registry::create()
{
    return { this, getReg().create<Components_t...>() };
}
/// @copydoc ecs::registry::create
template <typename... Components_t> 
inline Entity Registry::create(Components_t&&... components)
{
    return { this, getReg().create<Components_t...>(std::forward<Components_t...>(components...)) };
}
/// @copydoc ecs::registry::destroy
inline void Registry::destroy(Entity const &entity)
{
    getReg().destroy(entity.entity());
}
/// @copydoc ecs::registry::size
inline std::size_t Registry::size() const
{
    return getReg().size();
}
/// @copydoc ecs::registry::view
template<typename... Include, typename... Exclude>
inline std::vector<Entity> Registry::view(exclude<Exclude...> toExclude) const
{
    std::vector<Entity> res;
    for(auto const &e : getReg().view<Include...>(toExclude))
        res.emplace_back(const_cast<Registry *>(this), e); // Whats the worst that could happen :clueless:

    return res;
}
/// @copydoc ecs::registry::viewAny
template<typename... Include, typename... Exclude>
inline std::vector<Entity> Registry::viewAny(exclude<Exclude...> toExclude) const
{
    std::vector<Entity> res;
    for(auto const &e : getReg().viewAny<Include...>(toExclude))
        res.emplace_back(const_cast<Registry *>(this), e); // Please dont kill me for my sins

    return res;
}
/// @copydoc ecs::registry::merged
inline Registry Registry::merged(Registry const &other) const
{
    return Registry(getReg().merged(other.getReg()));
}
/// @copydoc ecs::registry::merged
inline void Registry::merge(Registry const &other)
{
    getReg().merge(other.getReg());
}

constexpr ecs::entity INVALID_ENTITY = 0;

namespace std {
    template<>
    struct hash<Entity> {
        size_t operator()(Entity const &e) const {
            size_t seed = std::hash<std::uintptr_t>{}(reinterpret_cast<std::uintptr_t>(&e.reg()));
            seed ^= std::hash<ecs::entity>{}(e.entity()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

template <typename T>
using SparseSet = ecs::sparse_set<T>;
