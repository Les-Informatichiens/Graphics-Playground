//
// Created by Jonathan Richard on 2024-04-30.
//

#pragma once

#include "engine/EngineInstance.h"
#include "engine/Mesh.h"
#include "engine/ResourceManager.h"
#include "engine/graphics/DeviceManager.h"
#include "engine/graphics/Material.h"
#include "engine/graphics/Renderer.h"


namespace TempResourceInitializer
{

    void loadObj(std::string file, Mesh& mesh);

    void init(ResourceManager& resourceManager, graphics::Renderer& renderer, const InstanceDesc& desc);
}