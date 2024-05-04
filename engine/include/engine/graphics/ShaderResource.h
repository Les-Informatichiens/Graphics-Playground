//
// Created by Jonathan Richard on 2024-03-29.
//

#pragma once


#include "engine/Resource.h"

#include <memory>

namespace graphics
{
    class ShaderProgram;
}

class ShaderResource : public Resource
{
public:
    ShaderResource(ResourceManager* manager, std::string name, ResourceHandle handle_, bool external);
    ~ShaderResource() override;

    void load() override;
    void loadFromManagedResource(std::shared_ptr<graphics::ShaderProgram> program);
    void unload() override;

    [[nodiscard]] std::shared_ptr<graphics::ShaderProgram> getShaderProgram() const;
    void setShaderProgram(std::shared_ptr<graphics::ShaderProgram> program);

private:
    std::shared_ptr<graphics::ShaderProgram> internalShaderProgram_;
};
