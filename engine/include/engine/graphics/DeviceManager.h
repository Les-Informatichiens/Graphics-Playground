//
// Created by Jonathan Richard on 2024-03-21.
//

#pragma once

#include "Material.h"
#include "ShaderProgram.h"
#include "VertexData.h"
#include "graphicsAPI/common/Device.h"
#include <memory>

namespace graphics
{

class DeviceManager
{
public:
    DeviceManager();

    void initialize(std::unique_ptr<IDevice> device);

    IDevice& getDevice() const;

    std::shared_ptr<Material> createMaterial(const std::shared_ptr<ShaderProgram>& shaderProgram);
    std::shared_ptr<ShaderProgram> createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);

    [[nodiscard]] std::shared_ptr<VertexData> createIndexedVertexData(const VertexDataLayout& layout, IndexFormat indexFormat, uint32_t vertexCount = 0, uint32_t indexCount = 0) const;
//    std::shared_ptr<VertexData> createCachedIndexedVertexData(size_t id, const VertexDataLayout& layout, IndexFormat indexFormat, uint32_t vertexCount = 0, uint32_t indexCount = 0);

    [[nodiscard]] std::shared_ptr<VertexData> createVertexData(const VertexDataLayout& layout, uint32_t vertexCount = 0) const;
//    std::shared_ptr<VertexData> createCachedVertexData(size_t id, const VertexDataLayout& layout, uint32_t vertexCount = 0);

private:
    std::unique_ptr<IDevice> device;
};

}// namespace graphics