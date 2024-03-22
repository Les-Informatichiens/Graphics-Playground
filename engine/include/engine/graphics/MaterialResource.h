//
// Created by Jonathan Richard on 2024-03-21.
//

#pragma once


#include "engine/Resource.h"

#include "Material.h"
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

    void unload() override
    {
        internalMaterial_.reset();
        setState(LoadingState::Unloaded);
    }

    [[nodiscard]] std::shared_ptr<graphics::Material> getMaterial() const
    {
        return internalMaterial_;
    }

    void setMaterial(std::shared_ptr<graphics::Material> material)
    {
        internalMaterial_ = std::move(material);
    }

private:
    std::shared_ptr<graphics::Material> internalMaterial_;

    MaterialMetadata metadata_;
};
