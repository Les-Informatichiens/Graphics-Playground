//
// Created by Jonathan Richard on 2024-03-12.
//

#pragma once

#include "engine/ImageResource.h"
#include "engine/Mesh.h"
#include "engine/ResourceHandle.h"
#include "engine/graphics/MeshResource.h"
#include "engine/graphics/ShaderResource.h"
#include "engine/graphics/TextureResource.h"

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
    std::unordered_map<std::string, std::shared_ptr<MaterialResource>> getMaterials() { return materialsByName; }
    void releaseMaterial(ResourceHandle handle);

    //image
    std::shared_ptr<ImageResource> createImage(const std::string& name, ImageResource::Format format = ImageResource::Format::RGBA);
    std::shared_ptr<ImageResource> createExternalImage(const std::string& name, ImageResource::Format format = ImageResource::Format::RGBA);
    std::shared_ptr<ImageResource> getImageByHandle(ResourceHandle handle);
    std::shared_ptr<ImageResource> getImageByName(const std::string& name);
    void releaseImage(ResourceHandle handle);

    //texture
    std::shared_ptr<TextureResource> createTexture(const std::string& name);
    std::shared_ptr<TextureResource> getTextureByHandle(ResourceHandle handle);
    std::shared_ptr<TextureResource> getTextureByName(const std::string& name);
    std::unordered_map<std::string, std::shared_ptr<TextureResource>> getTextures() { return texturesByName; }
    void releaseTexture(ResourceHandle handle);

    //shader
    std::shared_ptr<ShaderResource> createShader(const std::string& name);
    std::shared_ptr<ShaderResource> createExternalShader(const std::string& name);
    std::shared_ptr<ShaderResource> getShaderByHandle(ResourceHandle handle);
    std::shared_ptr<ShaderResource> getShaderByName(const std::string& name);
    void releaseShader(ResourceHandle handle);

private:
    ResourceHandle getNewHandle();

private:
    std::unordered_map<ResourceHandle, std::shared_ptr<MeshResource>> meshesByHandle;
    std::unordered_map<std::string, std::shared_ptr<MeshResource>> meshesByName;

    std::unordered_map<ResourceHandle, std::shared_ptr<MaterialResource>> materialsByHandle;
    std::unordered_map<std::string, std::shared_ptr<MaterialResource>> materialsByName;

    //image
    std::unordered_map<ResourceHandle, std::shared_ptr<ImageResource>> imagesByHandle;
    std::unordered_map<std::string, std::shared_ptr<ImageResource>> imagesByName;

    //texture
    std::unordered_map<ResourceHandle, std::shared_ptr<TextureResource>> texturesByHandle;
    std::unordered_map<std::string, std::shared_ptr<TextureResource>> texturesByName;

    //shaders
    std::unordered_map<ResourceHandle, std::shared_ptr<ShaderResource>> shadersByHandle;
    std::unordered_map<std::string, std::shared_ptr<ShaderResource>> shadersByName;

    // more resource types

    ResourceHandle nextId = ResourceHandle(1);
};
