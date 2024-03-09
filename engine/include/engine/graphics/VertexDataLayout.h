//
// Created by Jonathan Richard on 2024-03-07.
//

#pragma once

#include "engine/util/Math.h"
#include "graphicsAPI/common/VertexInputState.h"
#include <cstdint>
#include <utility>

namespace graphics {


struct VertexAttribute
{
    std::string name;
    uint32_t location;
    VertexAttributeFormat format;
};

/**
 * This class is a simplified descriptor for the VertexInputState
 * It is used to describe the layout of the vertex data that will be used by the vertex shader
 * in a user friendly way.
 */
class VertexDataLayout
{
public:
    VertexDataLayout(std::vector<VertexAttribute> attributes) : attributes(std::move(attributes)) { calculateStride(); }

    [[nodiscard]] const std::vector<VertexAttribute>& getAttributes() const { return attributes; }
    uint32_t getStride() const { return stride; }

    void addAttribute(const std::string& name, uint32_t location, VertexAttributeFormat format)
    {
        attributes.push_back({name, location, format});
        calculateStride();
    }

private:
    void calculateStride()
    {
        stride = 0;
        for (const auto& attribute: attributes)
        {
            stride += getVertexAttributeFormatSize(attribute.format);
        }
    };

private:
    std::vector<VertexAttribute> attributes;
    uint32_t stride{};
};

struct VertexDataLayoutHash
{
    std::size_t operator()(const VertexDataLayout& layout) const
    {
        std::size_t hash = 0;
        for (const auto& attribute : layout.getAttributes())
        {
            util::Math::hash_combine(hash, std::hash<std::string>{}(attribute.name));
            util::Math::hash_combine(hash, std::hash<uint32_t>{}(attribute.location));
            util::Math::hash_combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(attribute.format)));
        }
        return hash;
    }
};

}// namespace graphics
