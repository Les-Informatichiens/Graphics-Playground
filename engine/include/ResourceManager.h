//
// Created by Jonathan Richard on 2024-03-12.
//

#pragma once

#include "engine/Mesh.h"

#include <string>
#include <unordered_map>
#include <algorithm>
#include <memory>


struct ResourceManagerDesc
{

};

class ResourceHandle
{
public:

//    struct NullHandle
//    {
//        template<typename T>
//        [[nodiscard]] constexpr bool operator==(const T& other) const noexcept {
//            return other == *this;
//        }
//
//        template<typename T>
//        [[nodiscard]] constexpr bool operator!=(const T& other) const noexcept {
//            return !(other == *this);
//        }
//
//        operator ResourceHandle() const noexcept {
//            return ResourceHandle(0);
//        }
//    };

public:
    using id_t = size_t;

    explicit ResourceHandle() : id(NullId) {}
    explicit ResourceHandle(id_t id) : id(id) {}

    id_t getId() const { return id; }

    bool isNull() const { return id == NullId; }

    operator bool() const { return !isNull(); }

    bool operator==(const ResourceHandle& other) const { return id == other.id; }
    bool operator!=(const ResourceHandle& other) const { return id != other.id; }

//    bool operator==(const NullHandle& other) const { return id == 0; }
//    bool operator!=(const NullHandle& other) const { return id != 0; }

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

class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    void initialize(const ResourceManagerDesc& desc);

    ResourceHandle loadMesh(const std::string& path);
    std::shared_ptr<Mesh> getMesh(ResourceHandle handle);
    // more resource types

    ResourceHandle addMesh(std::unique_ptr<Mesh> mesh);
    void updateMesh(ResourceHandle handle, std::unique_ptr<Mesh> mesh);


    void release(ResourceHandle handle);

private:
    std::unordered_map<ResourceHandle::id_t, std::shared_ptr<Mesh>> meshes;
    // more resource types

    ResourceHandle::id_t nextId = 1;
};
