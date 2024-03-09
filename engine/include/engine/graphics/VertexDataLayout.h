//
// Created by Jonathan Richard on 2024-03-07.
//

#pragma once

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

}// namespace graphics
