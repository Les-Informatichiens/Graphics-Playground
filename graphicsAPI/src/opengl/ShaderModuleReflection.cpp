//
// Created by Jonathan Richard on 2024-02-04.
//

#include "ShaderModuleReflection.h"
#include "util/hashed_string.h"

namespace opengl {

ShaderModuleReflection::ShaderModuleReflection(spirv_cross::Compiler& compiler)
{
    parsePushConstants(compiler);
    parseResources(compiler);
    parseInputAttributes(compiler);
}

void ShaderModuleReflection::parsePushConstants(spirv_cross::Compiler& compiler)
{
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    for (auto& resource: resources.push_constant_buffers)
    {
        spirv_cross::SPIRType resourceType = compiler.get_type(resource.base_type_id);
        for (uint32_t i = 0; i < resourceType.member_types.size(); ++i)
        {
            std::string name = (!resource.name.empty() ? (resource.name + ".") : "") + compiler.get_member_name(resourceType.self, i);
            spirv_cross::SPIRType memberType = compiler.get_type(resourceType.member_types[i]);

            OpenGLUniform uniform;
            uniform.name = name;
            uniform.type = SPIRTypeToDataType(memberType);
            uniform.location = static_cast<int32_t>(compiler.get_member_decoration(resource.base_type_id, i, spv::Decoration::DecorationLocation));
            uniform.offset = compiler.type_struct_member_offset(resourceType, i);
            uniform.size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(resourceType, i));
            pushConstants.try_emplace(entt::hashed_string(name.c_str()), std::move(uniform));
        }
    }
}

void ShaderModuleReflection::parseResources(spirv_cross::Compiler& compiler)
{
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    const std::vector<const OpenGLResourceMap*> otherResourceMapsUniform = {&samplerResources, &storageImageResources};
    const std::vector<const OpenGLResourceMap*> otherResourceMapsSampler = {&uniformBufferResources, &storageImageResources};
    const std::vector<const OpenGLResourceMap*> otherResourceMapsStorage = {&uniformBufferResources, &samplerResources};
    parseResourceBindingsInternal("Uniform Buffer", compiler, uniformBufferBindingMap, uniformBufferResources, otherResourceMapsUniform, resources.uniform_buffers);
    parseResourceBindingsInternal("Sampler", compiler, samplerBindingMap, samplerResources, otherResourceMapsSampler, resources.separate_samplers);
    parseResourceBindingsInternal("Storage Image", compiler, storageImageBindingMap, storageImageResources, otherResourceMapsStorage, resources.storage_images);
}


void ShaderModuleReflection::parseResourceBindingsInternal(const std::string_view& resourceType, spirv_cross::Compiler& compiler, OpenGLBindingMap& bindingMap, OpenGLResourceMap& resourceMap, const std::vector<const OpenGLResourceMap*>& otherResourceMaps, spirv_cross::SmallVector<spirv_cross::Resource>& resources)
{
    uint32_t numResources = 0;
    for (const auto binding : bindingMap) {
        numResources += binding.second.second;
    }

    for (const auto& resource : resources) {
        const auto& name = resource.name;
        const auto& type = compiler.get_type(resource.type_id);
        const auto& baseType = compiler.get_type(resource.base_type_id);
        std::vector<uint32_t> shape;
        uint32_t count = 1;
        if (!type.array.empty()) {
            shape.resize(type.array.size());  // number of dimensions of the array. 0 = its a scalar (i.e. not an array), 1 = 1D array, 2 = 2D array, etc...
            for (auto dim = 0; dim < shape.size(); ++dim) {
                shape[dim] = type.array[dim];  // size of [dim]th dimension of the array
                count *= type.array[dim];
            }
        }

        uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

//        PKZL_CORE_LOG_TRACE("Found {0} at set {1}, binding {2} with name '{3}', count is {4}", resourceType, set, binding, name, count);
        bindingMap.try_emplace(std::make_pair(set, binding), std::make_pair(numResources, count));

        auto [openGLBinding, dummy] = bindingMap.at({set, binding});
        compiler.set_decoration(resource.id, spv::DecorationDescriptorSet, 0);
        compiler.set_decoration(resource.id, spv::DecorationBinding, openGLBinding);
        numResources += count;

        const uint32_t id = entt::hashed_string(name.data());
        for (const auto otherResourceMap : otherResourceMaps) {
            const auto otherResource = otherResourceMap->find(id);
            if (otherResource != otherResourceMap->end()) {
                throw std::runtime_error {/*fmt::format*/("Shader resource name '{}' is ambiguous.  Refers to different types of resource!", name)};
            }
        }

        const auto openGLResource = resourceMap.find(id);
        if (openGLResource == resourceMap.end()) {
            resourceMap.emplace(entt::hashed_string(name.data()), OpenGLResource {name, openGLBinding, shape});
        } else {
            // already seen this name, check that binding is the same
            if (openGLResource->second.binding != openGLBinding) {
                throw std::runtime_error {/*fmt::format*/("Shader resource name '{}' is ambiguous.  Refers to different bindings!", name)};
            }
        }
    }
}

}// namespace opengl
