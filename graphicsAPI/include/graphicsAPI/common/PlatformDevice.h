//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

enum class PlatformDeviceType {
    Unknown = 0,
    Metal,
    OpenGL,
    Vulkan,
};

class IPlatformDevice
{
    friend class IDevice;

protected:
    IPlatformDevice() = default;
    virtual bool isOfType(PlatformDeviceType type) const = 0;

public:
    virtual ~IPlatformDevice() = default;
};
