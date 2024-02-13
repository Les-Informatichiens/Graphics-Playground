//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include <string>

enum class ShaderModuleType
{
    Vertex,
    Geometry,
    Fragment,
    Compute
};

struct ShaderModuleDesc
{
    ShaderModuleType type;
    std::string code;
    std::string entryPoint;
};

class IShaderModule
{
public:
    explicit IShaderModule(ShaderModuleDesc desc);
    virtual ~IShaderModule() = default;

    [[nodiscard]] ShaderModuleType getType() const;

protected:
    ShaderModuleDesc desc;
};
