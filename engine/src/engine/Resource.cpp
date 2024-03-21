//
// Created by Jonathan Richard on 2024-03-20.
//

#include "engine/Resource.h"

#include "engine/ResourceManager.h"
#include <utility>

Resource::Resource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external)
    : manager(manager), name(std::move(name)), handle(handle_), external(external)
{
}

Resource::~Resource() = default;

const std::string& Resource::getName()
{
    return name;
}

ResourceHandle Resource::getHandle()
{
    return handle;
}

bool Resource::isLoaded() const
{
    return state == LoadingState::Loaded;
}