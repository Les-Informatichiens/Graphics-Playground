//
// Created by Jonathan Richard on 2024-03-20.
//

#pragma once


#include "ResourceHandle.h"

#include <string>

class ResourceManager;

class Resource
{
public:
    enum class LoadingState
    {
        Error,
        Unloaded,
        Loading,
        Loaded,
    };

    explicit Resource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external);
    virtual ~Resource();

    virtual void load() = 0;
    virtual void unload() = 0;

    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] bool isExternal() const { return external; }

    const std::string& getName();
    ResourceHandle getHandle();

protected:
    void setState(LoadingState newState) { state = newState; }

private:
    ResourceManager* manager;
    std::string name;
    ResourceHandle handle;
    LoadingState state = LoadingState::Unloaded;
    const bool external;
};
