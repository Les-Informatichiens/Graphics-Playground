//
// Created by Jonathan Richard on 2024-02-04.
//

#include "GraphicsPipelineReflection.h"
#include "ShaderModule.h"
#include "fmt/format.h"

namespace opengl {

GraphicsPipelineReflection::GraphicsPipelineReflection(Context& context, const PipelineShaderStages& desc)
{
    auto vertShader = dynamic_cast<ShaderModule*>(desc.getVertexShader().get());
    auto fragShader = dynamic_cast<ShaderModule*>(desc.getFragmentShader().get());
    auto geomShader = dynamic_cast<ShaderModule*>(desc.getGeometryShader().get());

    if (vertShader)
    {
        reflectShader(vertShader->getReflection());
    }
    if (fragShader)
    {
        reflectShader(fragShader->getReflection());
    }
    if (geomShader)
    {
        reflectShader(geomShader->getReflection());
    }

}

void GraphicsPipelineReflection::reflectShader(const std::shared_ptr<ShaderModuleReflection>& reflection)
{
    // append all the shader reflection data to the pipeline reflection
    for (const auto& [k, v] : reflection->getUniformBufferBindingMap())
    {
        uniformBufferBindingMap.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getStorageImageBindingMap())
    {
        storageImageBindingMap.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getSamplerBindingMap())
    {
        samplerBindingMap.try_emplace(k, v);
    }

    for (const auto& [k, v] : reflection->getUniformBufferResources())
    {
        uniformBufferResources.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getStorageImageResources())
    {
        storageImageResources.try_emplace(k, v);
    }
    for (const auto& [k, v] : reflection->getSamplerResources())
    {
        samplerResources.try_emplace(k, v);
    }


    for (const auto& [k, v] : reflection->getPushConstants())
    {
        pushConstants.try_emplace(k, v);
    }
}

GLuint GraphicsPipelineReflection::getSamplerBinding(uint32_t resourceId, bool exceptionIfNotFound) const
{
    const auto resource = uniformBufferResources.find(resourceId);
    GLuint retVal = (resource == uniformBufferResources.end()) ? ~0 : resource->second.binding;
    if (exceptionIfNotFound && retVal == ~0) {
        throw std::runtime_error(fmt::format("Sampler resource not found: {}", resourceId));
    }
    return retVal;
}

GLuint GraphicsPipelineReflection::getStorageImageBinding(uint32_t resourceId, bool exceptionIfNotFound) const
{
    const auto resource = storageImageResources.find(resourceId);
    GLuint retVal = (resource == storageImageResources.end()) ? ~0 : resource->second.binding;
    if (exceptionIfNotFound && retVal == ~0) {
        throw std::runtime_error(fmt::format("Storage image resource not found: {}", resourceId));
    }
    return retVal;
}

GLuint GraphicsPipelineReflection::getUniformBufferBinding(uint32_t resourceId, bool exceptionIfNotFound) const
{
    const auto resource = uniformBufferResources.find(resourceId);
    GLuint retVal = (resource == uniformBufferResources.end()) ? ~0 : resource->second.binding;
    if (exceptionIfNotFound && retVal == ~0) {
        throw std::runtime_error(fmt::format("Uniform buffer resource not found: {}", resourceId));
    }
    return retVal;
}

}// namespace opengl