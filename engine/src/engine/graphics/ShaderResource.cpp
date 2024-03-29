//
// Created by Jonathan Richard on 2024-03-29.
//

#include "engine/graphics/ShaderResource.h"

ShaderResource::ShaderResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external)
    : Resource(manager, name, handle_, external)
{
}

ShaderResource::~ShaderResource() = default;

void ShaderResource::load()
{
    setState(LoadingState::Loaded);
}

void ShaderResource::unload()
{
    internalShaderProgram_.reset();
    setState(LoadingState::Unloaded);
}

void ShaderResource::setShaderProgram(std::shared_ptr<graphics::ShaderProgram> program)
{
    internalShaderProgram_ = std::move(program);
}

std::shared_ptr<graphics::ShaderProgram> ShaderResource::getShaderProgram() const
{
    return internalShaderProgram_;
}

void ShaderResource::loadFromManagedResource(std::shared_ptr<graphics::ShaderProgram> program)
{
    internalShaderProgram_ = std::move(program);
    setState(LoadingState::Loaded);
}
