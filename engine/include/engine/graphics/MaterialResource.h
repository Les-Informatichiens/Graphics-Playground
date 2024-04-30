//
// Created by Jonathan Richard on 2024-03-21.
//

#pragma once


#include "engine/Resource.h"

#include "Material.h"
#include "Renderer.h"
#include "TextureResource.h"
#include "ShaderResource.h"
#include <memory>
#include <utility>

struct MaterialMetadata
{
    std::string name;
    std::string program; // program resource name
    std::string albedo; // texture name
    std::string normal; // texture name
    std::string metallic; // texture name
    std::string roughness; // texture name
};

class MaterialResource : public Resource
{
    struct UniformBufferDesc
    {
        void* data;
        size_t size = 0;
        size_t bindingPoint = 0;
    };

    struct TextureSamplerDesc
    {
        size_t index = 0;
        std::shared_ptr<TextureResource> textureResource;
    };

public:
    MaterialResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external)
        : Resource(manager, std::move(name), handle_, external)
    {
    }

    void load() override
    {
        // there is nothing to load for a material
        setState(LoadingState::Loaded);
    }

    void loadFromManagedResource(std::shared_ptr<graphics::Material> material)
    {
        internalMaterial_ = std::move(material);
        setState(LoadingState::Loaded);
    }

    void unload() override
    {
        internalMaterial_.reset();
        shaderResource_.reset();
        uniformBuffers.clear();
        textureSamplers.clear();

        setState(LoadingState::Unloaded);
    }

    void use(graphics::Renderer& renderer)
    {
        for (const auto& [name, desc] : uniformBuffers)
        {
            internalMaterial_->setUniformBytes(name, desc.data, desc.size, desc.bindingPoint);
        }

        for (const auto& [name, desc] : textureSamplers)
        {
            internalMaterial_->setTextureSampler(name, desc.textureResource->getTexture(), desc.textureResource->getSamplerState(), desc.index);
        }
    }

    void setUniformBuffer(const std::string& name, void* data, size_t size, size_t bindingPoint)
    {
        UniformBufferDesc desc;
        desc.data = nullptr;
        desc.size = size;
        desc.bindingPoint = bindingPoint;

        // to ensure that the data is not deallocated before the material is destroyed
        // the material now owns a copy of the data
        desc.data = malloc(size);
        memcpy(desc.data, data, size);

        uniformBuffers[name] = desc;
    }

    void setTextureSampler(const std::string& name, std::shared_ptr<TextureResource> textureResource, size_t index)
    {
        TextureSamplerDesc desc;
        desc.index = index;
        desc.textureResource = textureResource;

        textureSamplers[name] = desc;
    }

    [[nodiscard]] std::shared_ptr<graphics::Material> getMaterial() const
    {
        return internalMaterial_;
    }

    void setMaterial(std::shared_ptr<graphics::Material> material)
    {
        internalMaterial_ = std::move(material);
    }

    void setShader(std::shared_ptr<ShaderResource> shader)
    {
        shaderResource_ = shader;
        internalMaterial_->setShaderProgram(shader->getShaderProgram());
    }

    [[nodiscard]] std::shared_ptr<ShaderResource> getShader() const
    {
        return shaderResource_.lock();
    }

private:
    std::shared_ptr<graphics::Material> internalMaterial_;

    std::weak_ptr<ShaderResource> shaderResource_;

    std::unordered_map<std::string, UniformBufferDesc> uniformBuffers;
    std::unordered_map<std::string, TextureSamplerDesc> textureSamplers;

    MaterialMetadata metadata_;
};
