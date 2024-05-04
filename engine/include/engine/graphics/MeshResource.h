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

/*
 * TODO: What to do with the meshes in the engine?
 * logically the next step would be to have some kind of prefab resource that contains a hierarchy of meshes and materials, probably in the form of a scene graph with components
 * this is really the best way i can think of being able to load models that are made up of multiple meshes and materials
 * from FBX or OBJ files. Assimp is a good library to extract all of the nodes from a model file.
 *
 * Should there just be a generic prefab resource type that can describe anything we can find in a scene? that way we could serialize and deserialize the entire scene graph or parts of it at wanted scene node level
 * If we can come up with some prefab resource, how would it be able to reference other resources? would it be by name? or by pointer?
 * If it's by name, how would we be able to resolve the references when loading the prefab? would we have to load all resources before loading the prefab?
 */
class MeshResource : public Resource
{

public:
    MeshResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external)
        : Resource(manager, std::move(name), handle_, external), internalMesh_(), vertexData_(nullptr), metadata_()
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
//        material_.reset();
        internalMesh_.clear();
        vertexData_.reset();
        metadata_ = MeshMetadata();

        setState(LoadingState::Unloaded);
    }
//
//    void setMaterial(const std::shared_ptr<MaterialResource>& material)
//    {
//        material_ = material;
//    }
//
//    std::shared_ptr<MaterialResource> getMaterial() const
//    {
//        return material_.lock();
//    }

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
//    std::weak_ptr<MaterialResource> material_;
};
