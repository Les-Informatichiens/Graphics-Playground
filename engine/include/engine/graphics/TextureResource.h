//
// Created by Jonathan Richard on 2024-03-20.
//

#pragma once


#include "engine/Resource.h"

#include "graphicsAPI/common/Texture.h"
#include "graphicsAPI/common/SamplerState.h"

#include <memory>

class TextureResource : public Resource
{
public:
    TextureResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external);
    ~TextureResource() override;

    void load() override;

    void loadFromManagedResource(std::shared_ptr<ITexture> texture, std::shared_ptr<ISamplerState> samplerState);

    void unload() override;

    [[nodiscard]] std::shared_ptr<ITexture> getTexture() const;
    void setTexture(std::shared_ptr<ITexture> texture);

    [[nodiscard]] std::shared_ptr<ISamplerState> getSamplerState() const;
    void setSamplerState(std::shared_ptr<ISamplerState> samplerState);

private:
    std::shared_ptr<ITexture> internalTexture_;
    std::shared_ptr<ISamplerState> internalSamplerState_;
};
