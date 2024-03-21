//
// Created by Jonathan Richard on 2024-03-12.
//

#include "engine/ResourceManager.h"

void ResourceManager::initialize(const ResourceManagerDesc& desc)
{

}

std::shared_ptr<MeshResource> ResourceManager::createMesh(const std::string& name)
{
    auto handle = getNewHandle();
    auto mesh = std::make_shared<MeshResource>(this, name, handle, false);
    meshesByName[name] = mesh;
    meshesByHandle[handle] = mesh;
    return mesh;
}

std::shared_ptr<MeshResource> ResourceManager::createExternalMesh(const std::string& name)
{
    auto handle = getNewHandle();
    auto mesh = std::make_shared<MeshResource>(this, name, handle, true);
    meshesByName[name] = mesh;
    meshesByHandle[handle] = mesh;
    return mesh;
}

std::shared_ptr<MeshResource> ResourceManager::getMeshByHandle(ResourceHandle handle)
{
    auto it = meshesByHandle.find(handle);
    if (it != meshesByHandle.end())
    {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<MeshResource> ResourceManager::getMeshByName(const std::string& name)
{
    auto it = meshesByName.find(name);
    if (it != meshesByName.end())
    {
        return it->second;
    }
    return nullptr;
}

void ResourceManager::releaseMesh(ResourceHandle handle)
{
    auto it = meshesByHandle.find(handle);
    if (it != meshesByHandle.end())
    {
        it->second->unload();
        meshesByHandle.erase(it);
    }
}

std::shared_ptr<MaterialResource> ResourceManager::createMaterial(const std::string& name)
{
    auto handle = getNewHandle();
    auto material = std::make_shared<MaterialResource>(this, name, handle, false);
    materialsByName[name] = material;
    materialsByHandle[handle] = material;
    return material;
}
std::shared_ptr<MaterialResource> ResourceManager::createExternalMaterial(const std::string& name)
{
    auto handle = getNewHandle();
    auto material = std::make_shared<MaterialResource>(this, name, handle, true);
    materialsByName[name] = material;
    materialsByHandle[handle] = material;
    return material;
}

std::shared_ptr<MaterialResource> ResourceManager::getMaterialByHandle(ResourceHandle handle)
{
    auto it = materialsByHandle.find(handle);
    if (it != materialsByHandle.end())
    {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<MaterialResource> ResourceManager::getMaterialByName(const std::string& name)
{
    auto it = materialsByName.find(name);
    if (it != materialsByName.end())
    {
        return it->second;
    }
    return nullptr;
}

void ResourceManager::releaseMaterial(ResourceHandle handle)
{
    auto it = materialsByHandle.find(handle);
    if (it != materialsByHandle.end())
    {
        it->second->unload();
        materialsByHandle.erase(it);
    }
}

ResourceHandle ResourceManager::getNewHandle()
{
    return ResourceHandle(nextId.getId() + 1);
}