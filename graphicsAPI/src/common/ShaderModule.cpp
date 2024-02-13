//
// Created by Jonathan Richard on 2024-01-31.
//

#include "graphicsAPI/common/ShaderModule.h"

#include <utility>

IShaderModule::IShaderModule(ShaderModuleDesc desc)
    : desc(std::move(desc))
{
}

ShaderModuleType IShaderModule::getType() const
{
    return this->desc.type;
}

