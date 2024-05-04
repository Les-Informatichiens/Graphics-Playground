//
// Created by Jonathan Richard on 2024-03-20.
//

#pragma once

#include <functional>

class ResourceHandle
{
public:
    using id_t = size_t;

    explicit ResourceHandle() : id(NullId) {}
    explicit ResourceHandle(id_t id) : id(id) {}

    id_t getId() const { return id; }

    bool isNull() const { return id == NullId; }

    operator bool() const { return !isNull(); }

    bool operator==(const ResourceHandle& other) const { return id == other.id; }
    bool operator!=(const ResourceHandle& other) const { return id != other.id; }

private:
    static constexpr id_t NullId = 0;

    id_t id;
};

namespace std
{
template<>
struct hash<ResourceHandle>
{
    size_t operator()(const ResourceHandle& handle) const
    {
        return std::hash<ResourceHandle::id_t>()(handle.getId());
    }
};
}