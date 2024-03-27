//
// Created by Jonathan Richard on 2024-03-22.
//

#pragma once


#include "engine/Resource.h"

class ImageResource : public Resource
{
public:
    enum class Format
    {
        AUTO = 0,
        RGB,
        RGBA
    };

    ImageResource(ResourceManager* manager, const std::string& name, ResourceHandle handle_, bool external, Format format = Format::RGBA);

    void load() override;
    void unload() override;

    unsigned char* getData() const;
    int getWidth() const;
    int getHeight() const;
    int getChannels() const;

private:
    unsigned char* data_ = nullptr;
    int width_;
    int height_;
    int channels_;
    Format format_;
};
