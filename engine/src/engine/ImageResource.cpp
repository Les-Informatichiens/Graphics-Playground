//
// Created by Jonathan Richard on 2024-03-22.
//

#include "ImageResource.h"
#include "engine/stb_image.h"

ImageResource::ImageResource(ResourceManager* manager, const std::string& name, const std::string& path, ResourceHandle handle_, bool external)
    : Resource(manager, name, handle_, external)
{
}

void ImageResource::load()
{
    std::string path = getName();
    stbi_set_flip_vertically_on_load(true);
    data_ = stbi_load(path.c_str(), &width_, &height_, &channels_, 0);
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
