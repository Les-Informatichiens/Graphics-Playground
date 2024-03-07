//
// Created by Jonathan Richard on 2024-02-12.
//

#pragma once

#include "Material.h"
#include "graphicsAPI/common/CommandBuffer.h"
#include "graphicsAPI/common/Device.h"

namespace graphics {

class Renderable
{
public:
    Renderable(std::shared_ptr<Material> material);

    void setElementCount(uint32_t count);

    void setVertexData(std::shared_ptr<IBuffer> buffer, std::shared_ptr<IBuffer> indexBuffer, uint32_t elementCount_);

    void draw(IDevice& device, ICommandBuffer& cmdBuffer) const;

private:
    std::shared_ptr<IBuffer> vertexBuffer;
    std::shared_ptr<IBuffer> indexBuffer;

    std::shared_ptr<Material> material;

    uint32_t elementCount;
};

} // namespace graphics