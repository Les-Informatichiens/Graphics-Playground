//
// Created by Jonathan Richard on 2024-03-28.
//

#pragma once

#include "ShaderStage.h"
#include "graphicsAPI/common/Common.h"

#include <memory>
#include <string>
#include <unordered_map>

struct ComputePipelineDesc
{
    std::shared_ptr<IPipelineShaderStages> shaderStages;

    std::unordered_map<size_t, std::string> imagesMap;
    std::unordered_map<size_t, std::string> buffersMap;
};

class IComputePipeline
{
public:
    virtual ~IComputePipeline() = default;
};

struct ComputePipelineDescHash
{
    size_t operator()(const ComputePipelineDesc& desc) const
    {
        size_t hash = 0;
        hash_combine(hash, std::hash<std::shared_ptr<IPipelineShaderStages>>{}(desc.shaderStages));
        for (const auto& [unit, image] : desc.imagesMap)
        {
            hash_combine(hash, unit);
            hash_combine(hash, std::hash<std::string>{}(image));
        }
        for (const auto& [unit, buffer] : desc.buffersMap)
        {
            hash_combine(hash, unit);
            hash_combine(hash, std::hash<std::string>{}(buffer));
        }

        return hash;
    }
};