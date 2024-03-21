//
// Created by Jonathan Richard on 2024-03-20.
//

#pragma once


#include <utility>

#include "MaterialResource.h"
#include "VertexData.h"
#include "engine/Mesh.h"
#include "engine/Resource.h"


struct MeshMetadata
{
    std::string material;
};

class MeshResource : public Resource
{

public:
    MeshResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external)
        : Resource(manager, std::move(name), handle_, external), internalMesh_(), vertexData_(nullptr), metadata_(), material_()
    {
    }


    void load() override
    {
        if (isExternal())
        {
            try {
                setState(LoadingState::Loading);
                // Load from file
                setState(LoadingState::Loaded);
            }
            catch (...)
            {
                setState(LoadingState::Error);
                throw;
            }
        }
        else
        {
            // Load from memory
        }

        setState(LoadingState::Loaded);
    }

    void unload() override
    {
        material_.reset();
        internalMesh_.clear();
        vertexData_.reset();
        metadata_ = MeshMetadata();

        setState(LoadingState::Unloaded);
    }

    void setMaterial(const std::shared_ptr<MaterialResource>& material)
    {
        material_ = material;
    }

    std::shared_ptr<MaterialResource> getMaterial() const
    {
        return material_.lock();
    }

    Mesh& getMesh()
    {
        return internalMesh_;
    }

    void setVertexData(std::shared_ptr<graphics::VertexData> vertexData)
    {
        vertexData_ = std::move(vertexData);
    }

    std::shared_ptr<graphics::VertexData> getVertexData() const
    {
        return vertexData_;
    }

private:
    Mesh internalMesh_;
    std::shared_ptr<graphics::VertexData> vertexData_;

    MeshMetadata metadata_;
    std::weak_ptr<MaterialResource> material_;
};
