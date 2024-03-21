//
// Created by Jonathan Richard on 2024-03-21.
//

#include "engine/graphics/DeviceManager.h"

namespace graphics {

DeviceManager::DeviceManager()
    : device(nullptr)
{
}

void DeviceManager::initialize(std::unique_ptr<IDevice> device_)
{
    this->device = std::move(device_);
}

IDevice& DeviceManager::getDevice() const
{
    if (this->device == nullptr)
    {
        throw std::runtime_error("The device has not been initialized yet.");
    }
    return *this->device;
}


std::shared_ptr<ShaderProgram> DeviceManager::createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
    auto& device_ = getDevice();
    auto vertexShader = device_.createShaderModule({
            .type = ShaderModuleType::Vertex,
            .code = vertexShaderSource,
            .entryPoint = "main"
    });
    auto fragmentShader = device_.createShaderModule({
            .type = ShaderModuleType::Fragment,
            .code = fragmentShaderSource,
            .entryPoint = "main"
    });
    auto vis = device_.createVertexInputState({});

    return std::make_shared<ShaderProgram>(getDevice(), fragmentShader, vertexShader, vis);
}

std::shared_ptr<Material> DeviceManager::createMaterial(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    auto material = std::make_shared<Material>(getDevice(), shaderProgram);
    return material;
}

std::shared_ptr<VertexData> DeviceManager::createIndexedVertexData(const VertexDataLayout& layout, IndexFormat indexFormat, uint32_t vertexCount, uint32_t indexCount) const
{
    return std::make_shared<VertexData>(getDevice(), layout, indexFormat, vertexCount, indexCount);
}
//std::shared_ptr<VertexData> DeviceManager::createCachedVertexData(size_t id, const VertexDataLayout& layout, uint32_t vertexCount)
//{
//    auto it = vertexDataCache.find(id);
//    if (it != vertexDataCache.end())
//    {
//        return it->second;
//    }
//    auto vertexData = createVertexData(layout, vertexCount);
//    vertexDataCache[id] = vertexData;
//    return vertexData;
//}
//std::shared_ptr<VertexData> DeviceManager::createCachedIndexedVertexData(size_t id, const VertexDataLayout& layout, IndexFormat indexFormat, uint32_t vertexCount, uint32_t indexCount)
//{
//    auto it = vertexDataCache.find(id);
//    if (it != vertexDataCache.end())
//    {
//        return it->second;
//    }
//    auto vertexData = createIndexedVertexData(layout, indexFormat, vertexCount, indexCount);
//    vertexDataCache[id] = vertexData;
//    return vertexData;
//}
std::shared_ptr<VertexData> DeviceManager::createVertexData(const VertexDataLayout& layout, uint32_t vertexCount) const
{
    return std::make_shared<VertexData>(getDevice(), layout, IndexFormat::UInt32, vertexCount, 0);
}

}// namespace graphics
