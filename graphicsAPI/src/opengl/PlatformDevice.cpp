//
// Created by Jonathan Richard on 2024-02-03.
//

#include "graphicsAPI/opengl/PlatformDevice.h"

namespace opengl
{

    bool PlatformDevice::isOfType(PlatformDeviceType type) const
    {
        return type == PlatformDeviceType::OpenGL;
    }
} // namespace opengl
