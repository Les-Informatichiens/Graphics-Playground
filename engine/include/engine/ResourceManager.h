//
// Created by Jonathan Richard on 2024-03-12.
//

#pragma once

#include "engine/Mesh.h"
#include "engine/ResourceHandle.h"
#include "engine/graphics/MeshResource.h"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>


struct ResourceManagerDesc
{
};

class Resource;

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

    std::shared_ptr<MeshResource> createMesh(const std::string& name);
    std::shared_ptr<MeshResource> createExternalMesh(const std::string& name);
    std::shared_ptr<MeshResource> getMeshByHandle(ResourceHandle handle);
    std::shared_ptr<MeshResource> getMeshByName(const std::string& name);
    void releaseMesh(ResourceHandle handle);

    std::shared_ptr<MaterialResource> createMaterial(const std::string& name);
    std::shared_ptr<MaterialResource> createExternalMaterial(const std::string& name);
    std::shared_ptr<MaterialResource> getMaterialByHandle(ResourceHandle handle);
    std::shared_ptr<MaterialResource> getMaterialByName(const std::string& name);
    void releaseMaterial(ResourceHandle handle);

private:
    ResourceHandle getNewHandle();

private:
    std::unordered_map<ResourceHandle, std::shared_ptr<MeshResource>> meshesByHandle;
    std::unordered_map<std::string, std::shared_ptr<MeshResource>> meshesByName;

    std::unordered_map<ResourceHandle, std::shared_ptr<MaterialResource>> materialsByHandle;
    std::unordered_map<std::string, std::shared_ptr<MaterialResource>> materialsByName;

    // more resource types

    ResourceHandle nextId = ResourceHandle(1);
};
