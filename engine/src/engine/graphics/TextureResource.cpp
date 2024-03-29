//
// Created by Jonathan Richard on 2024-03-20.
//

#include "engine/graphics/TextureResource.h"

#include "engine/ResourceManager.h"

TextureResource::TextureResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external)
    : Resource(manager, std::move(name), handle_, external)
{
}

TextureResource::~TextureResource() = default;

void TextureResource::load()
{
    setState(LoadingState::Loaded);
}

void TextureResource::loadFromManagedResource(std::shared_ptr<ITexture> texture, std::shared_ptr<ISamplerState> samplerState)
{
    internalTexture_ = std::move(texture);
    internalSamplerState_ = std::move(samplerState);
    setState(LoadingState::Loaded);
}

void TextureResource::unload()
{
    internalTexture_.reset();
    internalSamplerState_.reset();
    setState(LoadingState::Unloaded);
}

void TextureResource::setTexture(std::shared_ptr<ITexture> texture)
{
    internalTexture_ = std::move(texture);
}

std::shared_ptr<ITexture> TextureResource::getTexture() const
{
    return internalTexture_;
}

std::shared_ptr<ISamplerState> TextureResource::getSamplerState() const
{
    return internalSamplerState_;
}

void TextureResource::setSamplerState(std::shared_ptr<ISamplerState> samplerState)
{
    internalSamplerState_ = std::move(samplerState);
}
