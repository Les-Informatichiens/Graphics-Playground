//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once


#include "graphicsAPI/common/PlatformDevice.h"

namespace opengl {

class PlatformDevice : public IPlatformDevice
{
public:
    PlatformDevice() = default;

    [[nodiscard]] bool isOfType(PlatformDeviceType type) const override;
};

}
