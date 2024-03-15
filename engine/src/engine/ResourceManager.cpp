//
// Created by Jonathan Richard on 2024-03-12.
//

#include "ResourceManager.h"

void ResourceManager::initialize(const ResourceManagerDesc& desc)
{

}

ResourceHandle ResourceManager::loadMesh(const std::string& path)
{
    // not implemented
    return ResourceHandle();
}

std::shared_ptr<Mesh> ResourceManager::getMesh(ResourceHandle handle)
{
    if (handle.isNull())
    {
        return nullptr;
    }

    if (auto it = meshes.find(handle); it != meshes.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

ResourceHandle ResourceManager::addMesh(std::unique_ptr<Mesh> mesh)
{
    ResourceHandle handle = ResourceHandle(nextId++);
    meshes[handle.getId()] = std::move(mesh);
    return handle;
}

void ResourceManager::updateMesh(ResourceHandle handle, std::unique_ptr<Mesh> mesh)
{
    if (handle.isNull())
    {
        return;
    }

    if (auto it = meshes.find(handle); it != meshes.end())
    {
        it->second = std::move(mesh);
    }
}

void ResourceManager::release(ResourceHandle handle)
{
    if (handle.isNull())
    {
        return;
    }

    if (auto it = meshes.find(handle); it != meshes.end())
    {
        meshes.erase(it);
    }
}
