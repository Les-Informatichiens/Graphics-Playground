//
// Created by Jonathan Richard on 2024-03-22.
//

#include "engine/ImageResource.h"
#include "engine/stb_image.h"

#include <iostream>

ImageResource::ImageResource(ResourceManager* manager, const std::string& name, ResourceHandle handle_, bool external, Format format)
    : Resource(manager, name, handle_, external), format_(format)
{
}

void ImageResource::load()
{
    std::string path = getName();
//    stbi_set_flip_vertically_on_load(true);

    auto stbiFormat = 0;
    switch (format_)
    {
        case Format::AUTO:
            break;
        case Format::RGB:
            stbiFormat = STBI_rgb;
            break;
        case Format::RGBA:
            stbiFormat = STBI_rgb_alpha;
            break;
    }

    data_ = stbi_load(path.c_str(), &width_, &height_, &channels_, stbiFormat);
    // error handling
    // get error from stbi_failure_reason()
    if (!data_)
    {
        if (const char* failure = stbi_failure_reason())
        {
            std::cerr << "Failed to load image: " << path << " Reason: " << failure << std::endl;
            return;
        }
        return;
    }

    setState(LoadingState::Loaded);
}

void ImageResource::unload()
{
    if (data_)
    {
        stbi_image_free(data_);
        data_ = nullptr;
    }
    setState(LoadingState::Unloaded);
}

unsigned char* ImageResource::getData() const
{
    return data_;
}

int ImageResource::getWidth() const
{
    return width_;
}

int ImageResource::getHeight() const
{
    return height_;
}

int ImageResource::getChannels() const
{
    return channels_;
}
