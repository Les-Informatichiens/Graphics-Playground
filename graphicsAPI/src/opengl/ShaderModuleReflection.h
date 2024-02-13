//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include "graphicsAPI/common/Common.h"

//#include <spirv_cross.hpp>
#include <unordered_map>
#include <string>
#include <vector>

namespace opengl {

struct OpenGLUniform
{
    std::string name;
    DataType type;
    int32_t location;
    uint32_t offset;
    uint32_t size;
};

struct OpenGLResource
{
    std::string name;
    uint32_t binding;
    std::vector<uint32_t> shape = {};
};

struct PairHash {
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& p) const {
        return std::hash<uint32_t>()(p.first) ^ std::hash<uint32_t>()(p.second);
    }
};

using OpenGLUniformMap = std::unordered_map<uint32_t, OpenGLUniform>;
using OpenGLBindingMap = std::unordered_map<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>, PairHash>;
using OpenGLResourceMap = std::unordered_map<uint32_t, OpenGLResource>;

class ShaderModuleReflection
{
public:
    ShaderModuleReflection() = default;
//    explicit ShaderModuleReflection(spirv_cross::Compiler& compiler);

    [[nodiscard]] const OpenGLUniformMap& getPushConstants() const { return pushConstants; }
    [[nodiscard]] const OpenGLBindingMap& getUniformBufferBindingMap() const { return uniformBufferBindingMap; }
    [[nodiscard]] const OpenGLResourceMap& getUniformBufferResources() const { return uniformBufferResources; }
    [[nodiscard]] const OpenGLBindingMap& getSamplerBindingMap() const { return samplerBindingMap; }
    [[nodiscard]] const OpenGLResourceMap& getSamplerResources() const { return samplerResources; }
    [[nodiscard]] const OpenGLBindingMap& getStorageImageBindingMap() const { return storageImageBindingMap; }
    [[nodiscard]] const OpenGLResourceMap& getStorageImageResources() const { return storageImageResources; }

private:
//    void parsePushConstants(spirv_cross::Compiler& compiler);
//    void parseResources(spirv_cross::Compiler& compiler);
//    void parseInputAttributes(spirv_cross::Compiler& compiler);

//    static void parseResourceBindingsInternal(const std::string_view& resourceType, spirv_cross::Compiler& compiler, OpenGLBindingMap& bindingMap, OpenGLResourceMap& resourceMap, const std::vector<const OpenGLResourceMap*>& otherResourceMaps, spirv_cross::SmallVector<spirv_cross::Resource>& resources);

    OpenGLUniformMap pushConstants;
    OpenGLBindingMap uniformBufferBindingMap;
    OpenGLResourceMap uniformBufferResources;// maps resource id (essentially the name of the resource) -> its opengl binding
    OpenGLBindingMap samplerBindingMap;
    OpenGLResourceMap samplerResources;// maps resource id (essentially the name of the resource) -> its opengl binding
    OpenGLBindingMap storageImageBindingMap;
    OpenGLResourceMap storageImageResources;
};

}// namespace opengl
